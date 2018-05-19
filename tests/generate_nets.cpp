// Copyright (c) 2018 S. D. Adams <s.d.adams.software@gmail.com>

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

using Swap = std::pair<size_t, size_t>;
using Swaps = std::vector<Swap>;

void P(Swaps &swaps, size_t i, size_t j)
{
    swaps.push_back(std::make_pair(i - 1, j - 1));
}

void Pbracket (Swaps &swaps,
               size_t i, /* value of first element in sequence 1 */
               size_t x, /* length of sequence 1 */
               size_t j, /* value of first element in sequence 2 */
               size_t y) /* length of sequence 2 */
{
    size_t a, b;

    if (x == 1 && y == 1)
    {
        P(swaps, i, j); /* 1 comparison sorts 2 items */
    }
    else if (x == 1 && y == 2)
    {
        /* 2 comparisons inserts an item into an
     * already sorted sequence of length 2. */
        P(swaps, i, (j + 1));
        P(swaps, i, j);
    }
    else if (x == 2 && y == 1)
    {
        /* As above, but inserting j */
        P(swaps, i, j);
        P(swaps, (i + 1), j);
    }
    else
    {
        /* Recurse on shorter sequences, attempting
     * to make the length of one subsequence odd
     * and the length of the other even. If we
     * can do this, we eventually merge the two. */
        a = x / 2;
        b = (x & 1) ? (y / 2) : ((y + 1) / 2);
        Pbracket(swaps, i, a, j, b);
        Pbracket(swaps, (i + a), (x - a), (j + b), (y - b));
        Pbracket(swaps, (i + a), (x - a), j, b);
    }
}

void Pstar (Swaps &swaps,
            size_t i, /* value of first element in sequence */
            size_t m) /* length of sequence */
{
    size_t a;

    if (m > 1)
    {
        /* Partition into 2 shorter sequences,
     * generate a sorting method for each,
     * and merge the two sub-networks. */
        a = m / 2;
        Pstar(swaps, i, a);
        Pstar(swaps, (i + a), (m - a));
        Pbracket(swaps, i, a, (i + a), (m - a));
    }
}

std::vector<std::pair<size_t, size_t>> bose_swaps(int n)
{
    std::vector<std::pair<size_t, size_t>> swaps;

    auto bose = [&](size_t n)
    {
        Pstar(swaps, 1, n); /* sort the sequence {X1,...,Xn} */
    };

    bose(n);

    return swaps;
}

void print_nets_abstract(int const max_count)
{
    size_t max_parallelism = 0;
    for (int i = 2; i <= max_count; ++i)
    {
        std::vector<std::pair<size_t, size_t>> const s = bose_swaps(i);
        std::vector<bool> r(i);
        for (int j = 0; j < i; ++j) r[j] = false;
        size_t m = 0;

        std::cout << "Bose Swaps " << i << std::endl;
        std::cout << "{";
        for (std::pair<size_t, size_t> p : s)
        {
            if (r[p.first] || r[p.second])
            {
                std::cout << "}" << std::endl << "{";
                for (int j = 0; j < i; ++j) r[j] = false;
                if (m > max_parallelism) max_parallelism = m;
                m = 0;
            }
            std::cout << "{" << p.first << ", " << p.second << "}, ";
            r[p.first] = true;
            r[p.second] = true;
            ++m;
        }
        std::cout << "}" << std::endl << std::endl;
    }
    std::cout << "max parallelism: " << max_parallelism << std::endl;
}

void print_nets_code(int const max_count)
{
    std::cout << "// Copyright (c) 2018 S. D. Adams <s.d.adams.software@gmail.com>" << std::endl << std::endl;

    std::cout << "#ifndef SORTING_NETWORK_HPP" << std::endl;
    std::cout << "#define SORTING_NETWORK_HPP" << std::endl << std::endl;

    std::cout << "// This file is automatically generated by tests/generate_nets.  Do not edit manually, changes will be lost." << std::endl << std::endl;

    std::cout << "#include \"../avx2_cas.hpp\"" << std::endl << std::endl;

    std::cout << "namespace sorting_network" << std::endl;
    std::cout << "{" << std::endl;
    std::cout << "    template <typename T> void sort(T *data, size_t n);" << std::endl << std::endl;
    std::cout << "    namespace internal" << std::endl;
    std::cout << "    {" << std::endl;
    std::cout << "        template <size_t const n> class net {public: template <typename T> static void sort(T *);};" << std::endl;
    std::cout << "    }" << std::endl;
    std::cout << "}" << std::endl << std::endl;

    for (int i = 2; i <= max_count; ++i)
    {
        std::vector<std::pair<size_t, size_t>> const s = bose_swaps(i);
        std::vector<bool> r(i);
        for (int j = 0; j < i; ++j) r[j] = false;

        std::vector<std::vector<size_t>> indices;
        indices.clear();
        indices.push_back(std::vector<size_t>());
        for (std::pair<size_t, size_t> p : s)
        {
            if (r[p.first] || r[p.second])
            {
                indices.push_back(std::vector<size_t>());
                for (int j = 0; j < i; ++j) r[j] = false;
            }

            indices.back().push_back(p.first);
            indices.back().push_back(p.second);
            r[p.first] = true;
            r[p.second] = true;
        }

        std::cout << "template <>" << std::endl;
        std::cout << "template <typename T>" << std::endl;
        std::cout << "inline" << std::endl;
        std::cout << "void" << std::endl;
        std::cout << "sorting_network::internal::net<" << i << ">::sort(T * const data)" << std::endl;
        std::cout << "{" << std::endl;
        for (std::vector<size_t> ind : indices)
        {
            switch (ind.size())
            {
            case 2:
                std::cout << "    avx2_cas::cas<T, "
                          << ind.at(0) << ", "
                          << ind.at(1)
                          << ">(data);" << std::endl;
                break;
            case 4:
                std::cout << "    avx2_cas::cas2<T>::template cas<"
                          << ind.at(0) << ", "
                          << ind.at(1) << ", "
                          << ind.at(2) << ", "
                          << ind.at(3)
                          << ">(data);" << std::endl;
                break;
            case 6:
                std::cout << "    avx2_cas::cas3<T>::template cas<"
                          << ind.at(0) << ", "
                          << ind.at(1) << ", "
                          << ind.at(2) << ", "
                          << ind.at(3) << ", "
                          << ind.at(4) << ", "
                          << ind.at(5)
                          << ">(data);" << std::endl;
                break;
            case 8:
                std::cout << "    avx2_cas::cas4<T>::template cas<"
                          << ind.at(0) << ", "
                          << ind.at(1) << ", "
                          << ind.at(2) << ", "
                          << ind.at(3) << ", "
                          << ind.at(4) << ", "
                          << ind.at(5) << ", "
                          << ind.at(6) << ", "
                          << ind.at(7)
                          << ">(data);" << std::endl;
                break;
            default:
                assert(false);
            }
        }
        std::cout << "}" << std::endl << std::endl;
    }

    std::cout << "template <typename T>" << std::endl;
    std::cout << "inline void sorting_network::sort(T * const data, size_t const n)" << std::endl;
    std::cout << "{" << std::endl;
    std::cout << "    if (n < 2)" << std::endl;
    std::cout << "    {" << std::endl;
    std::cout << "        return;" << std::endl;
    std::cout << "    }" << std::endl;
    std::cout << "    switch (n)" << std::endl;
    std::cout << "    {" << std::endl;
    for (int i = 2; i <= 32; ++i)
    {
        std::cout << "    case " << i << ":" << std::endl;
        std::cout << "        sorting_network::internal::net<" << i << ">::sort(data);" << std::endl;
        std::cout << "    break;" << std::endl;
    }
    std::cout << "    default: assert(false);" << std::endl;
    std::cout << "    }" << std::endl;
    std::cout << "}" << std::endl << std::endl;

    std::cout << "#endif // SORTING_NETWORK_HPP" << std::endl;
}

int main()
{
//    print_nets_abstract(32);
    print_nets_code(32);
    return 0;
}