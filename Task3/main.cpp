#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <ctime>
using namespace std;

template <class Stream>
void print_lines(int **a, int lines, int columns, Stream &out)
{
    for (int i = 1; i < lines; ++i)
    {
        for (int j = 0; j < columns; ++j)
            out << a[i][j] << '\t';
        out << '\n';
    }
}

int main(int argc, char **argv)
{
    int p, r, N1, N2;
    int myrank;
    int tag = 10;
    MPI_Status status;
    MPI_Request request1 = MPI_REQUEST_NULL, request2 = MPI_REQUEST_NULL;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    std::ofstream out{"logs/log" + std::to_string(myrank) + ".txt"};

    N1 = std::atoi(argv[1]);
    N2 = std::atoi(argv[2]);
    r = std::atoi(argv[3]);

    // TODO: more uniform distribution of lines
    int lines_number = N1 / p + (myrank < N1 % p);
    out << "lines_number: " << lines_number << std::endl;

    auto *memory = new int[lines_number * N2];
    auto **a = new int *[lines_number];

    for (int i = 0; i < lines_number; ++i)
    {
        a[i] = memory + i * N2;
    }

    srand(time(NULL));

    for (int jb = 0; jb < N2; jb += r)
    {
        int block_size = min(r, N2 - jb);

        if (myrank == 0)
        {
            for (int j = 0; j < jb + block_size; j++)
            {
                a[0][j] = (rand() % 100);
            }
        }
        else
        {
            if (jb == 0)
            {
                MPI_Irecv(a[0] + jb, block_size, MPI_INT, myrank - 1, tag, MPI_COMM_WORLD, &request1);
            }

            MPI_Wait(&request1, &status);
            if (jb + r < N2)
            {
                MPI_Irecv(a[0] + jb + r, block_size, MPI_INT, myrank - 1, tag, MPI_COMM_WORLD, &request1);
            }
        }

        for (int i = 1; i < lines_number; ++i)
        {
            for (int j = jb; j < jb + block_size; ++j)
            {
                a[i][j] = a[i - 1][j] + 1;
            }
        }

        if (myrank < p - 1)
        {
            if (request2 != MPI_REQUEST_NULL)
            {
                MPI_Wait(&request2, &status);
            }
            MPI_Isend(a[lines_number - 1] + jb, block_size, MPI_INT, myrank + 1, tag, MPI_COMM_WORLD, &request2);
        }
    }
    MPI_Wait(&request2, &status);

    int print_code = 1;
    if (myrank != 0)
    {
        MPI_Recv(&print_code, 1, MPI_INT, myrank - 1, tag, MPI_COMM_WORLD, &status);
    }

    std::cout << "Results for process #" << myrank << ":\n";
    print_lines(a, lines_number, N2, std::cout);
    if (myrank < p - 1)
    {
        MPI_Send(&print_code, 1, MPI_INT, myrank + 1, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}