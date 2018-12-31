#include <mpi.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
    double f(double x)
    {
        return x * x;
    }

    const int NO_BLOCK = -1;
    const int TAG = 10;
    const double a = 0;
    const double b = 5;
} // namespace

int main(int argc, char **argv)
{
    int p, r, n;
    int myrank;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    n = std::atoi(argv[1]);
    r = std::atoi(argv[2]);

    const auto dx = (b - a) / n;
    auto block_index = 0;
    auto result = 0.0;

    if (myrank == 0)
    {
        const int blocks_total = std::ceil(n * 1.0 / r);
        auto blocks_received = -p + 1;

        while (blocks_received < blocks_total)
        {
            double sum;
            MPI_Recv(&sum, 1, MPI_DOUBLE, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);

            if (block_index < blocks_total)
            {
                MPI_Send(&block_index, 1, MPI_INT, status.MPI_SOURCE, TAG, MPI_COMM_WORLD);
                block_index++;
            }
            else
            {
                MPI_Send(&NO_BLOCK, 1, MPI_INT, status.MPI_SOURCE, TAG, MPI_COMM_WORLD);
            }

            result += sum;
            ++blocks_received;
        }

        std::cout << "Result: " << result << std::endl;
    }
    else
    {
        MPI_Send(&result, 1, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD);
        MPI_Recv(&block_index, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &status);

        while (block_index != NO_BLOCK)
        {

            auto x = a + dx * block_index * r;
            const auto right = std::min(x + dx * r, b);

            auto result = f(x) / 2;

            while (x < right - dx)
            {
                x += dx;
                result += f(x);
            }
            result -= f(x) / 2;
            result *= dx;
            result += (right - x) * (f(x) + f(right)) / 2;

            std::cout << myrank << ": " << right << std::endl;
            MPI_Send(&result, 1, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD);
            MPI_Recv(&block_index, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &status);
        }
    }

    MPI_Finalize();
    return 0;
}