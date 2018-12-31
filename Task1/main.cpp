#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

    int p;
    int myrank;
    int str_size = 100;
    char str[100];
    int tag = 10;
    MPI_Status status;
    /* Инициализация библиотеки MPI*/
    MPI_Init(&argc, &argv);
    /* Каждая ветвь узнает количество задач в стартовавшем приложении */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    /* и свой собственный номер: от 0 до (size-1) */
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    /*Запускаем заданное число задач */
    if (myrank != 0)
    {
        std::cout << str << "Hello from process" << myrank << std::endl;
        /* Ветвь myrank  передает str ветви 0 */
        MPI_Send(str, (int)strlen(str) + 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
    }
    else
    {
        char **responses = (char **)malloc(p * sizeof(char *));

        int count, source;
        for (int i = 0; i < p - 1; i++)
        {
            /*функция, которая позволяет узнать о характеристиках сообщения ДО того,
			как сообщение будет помещено в приемный пользовательский буфер*/
            MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
            /*Она возвращает заполненную структуру MPI_Status и после нее можно вызвать MPI_Get_count*/

            /*возвращает количество фактически принятых данных*/
            MPI_Get_count(&status, MPI_CHAR, &count);

            /*следующий за MPI_Probe вызов MPI_Recv с теми же параметрами поместит в буфер пользователя
			именно то сообщение, которое было принято функцией MPI_Probe*/

            /*программа узнает фактические номер/идентификатор*/
            source = status.MPI_SOURCE;
            responses[source - 1] = (char *)malloc(count * sizeof(char));
            /* ветвь дожидается сообщения и помещает пришедшие данные в буфер */
            /* дожидается сообщения и помещает пришедшие данные в буфер responses[source]*/
            /*С одной стороны, мы передаем в MPI_Recv номер задачи, от которой ждем сообщение,
			и его идентификатор; а с другой - получаем их от MPI в структуре status*/
            MPI_Recv(responses[source - 1], count, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);

            // printf("Received message from process %d\n", source);
            std::cout << "Received message from process" << source << std::endl;
        }
        /*Выводим данные из буфера*/
        printf("\nResult:\n");
        for (int i = 0; i < p - 1; ++i)
        {
            // printf("%s\n", responses[i]);
            std::cout << responses[i] << std::endl;
        }

        for (int i = 0; i < p - 1; ++i)
        {
            free(responses[i]);
        }
        free(responses);
    }

    MPI_Finalize();
    return 0;

    /*Ветвью я называю процесс, извиняьюсь за свой "Французский"*/
}