#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MPI_Send(void* data, int count, MPI_Datatype datatype, int destination, int tag, MPI_Comm communicator)

int main (int argc, char *argv[]) {
    int numtasks, rank, len;
 
    int dimension, commError;

    int noWorkers;

    sscanf (argv[1],"%d",&dimension);
    sscanf (argv[2],"%d",&commError);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    // tag = 0 pt leader 0, 1 pt 1, 2 pt 2
    
    if (rank == 0) {
        FILE* inputFile = fopen("cluster0.txt", "r");
        int *ranks;
        int received1, *receivedVector1;
        int received2, *receivedVector2;
        
        MPI_Status status;

        fscanf(inputFile, "%d", &noWorkers);
        ranks = (int*)calloc(noWorkers, sizeof(int));
        
        for (int i = 0; i < noWorkers; i++)
            fscanf(inputFile, "%d", &ranks[i]);
            
        fclose(inputFile);
        
        // send to 1
        MPI_Send(&noWorkers, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkers, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);

        // send to 2
        MPI_Send(&noWorkers, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkers, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);

        // receive from 1
        MPI_Recv(&received1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
        receivedVector1 = (int*)calloc(received1, sizeof(int));
        MPI_Recv(receivedVector1, received1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);

        // receive from 2
        MPI_Recv(&received2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
        receivedVector2 = (int*)calloc(received2, sizeof(int));
        MPI_Recv(receivedVector2, received2, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);

        for (int i = 0; i < noWorkers; i++) {
            MPI_Send(&rank, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(&noWorkers, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(ranks, noWorkers, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(&received1, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            MPI_Send(receivedVector1, received1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            MPI_Send(&received2, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            MPI_Send(receivedVector2, received2, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, ranks[i]);
        }
        
        printf("0 -> ");
        printf("0:");
        for (int j = 0; j < noWorkers; j++) {
            if (j < noWorkers - 1) {
                printf("%d,",ranks[j]);
            } else {
                printf("%d ",ranks[j]);
            }
        }
        printf("1:");
        for (int j = 0; j < received1; j++) {
            if (j < received1 - 1) {
                printf("%d,",receivedVector1[j]);
            } else {
                printf("%d ",receivedVector1[j]);
            }
        }
        printf("2:");
        for (int j = 0; j < received2; j++) {
            if (j < received2 - 1) {
                printf("%d,",receivedVector2[j]);
            } else {
                printf("%d ",receivedVector2[j]);
            }
        }
        printf("\n");
    } else if (rank == 1) {
        FILE* inputFile = fopen("cluster1.txt", "r");
        int *ranks;
        int received0, *receivedVector0;
        int received2, *receivedVector2;

        MPI_Status status;

        fscanf(inputFile, "%d", &noWorkers);
        ranks = (int*)calloc(noWorkers, sizeof(int));
        
        for (int i = 0; i < noWorkers; i++)
            fscanf(inputFile, "%d", &ranks[i]);

        fclose(inputFile);

        // receive from 0
        MPI_Recv(&received0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        receivedVector0 = (int*)calloc(received0, sizeof(int));
        MPI_Recv(receivedVector0, received0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        // send to 0
        MPI_Send(&noWorkers, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkers, MPI_INT, 0, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);

        // send to 2
        MPI_Send(&noWorkers, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkers, MPI_INT, 2, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);

        // receive from 2
        MPI_Recv(&received2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
        receivedVector2 = (int*)calloc(received2, sizeof(int));
        MPI_Recv(receivedVector2, received2, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
        
        for (int i = 0; i < noWorkers; i++) {
            MPI_Send(&rank, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            MPI_Send(&noWorkers, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);            
            MPI_Send(ranks, noWorkers, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            MPI_Send(&received0, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(receivedVector0, received0, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(&received2, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            MPI_Send(receivedVector2, received2, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, ranks[i]);
        }
        
        printf("1 -> ");
        printf("0:");
        for (int j = 0; j < received0; j++) {
            if (j < received0 - 1) {
                printf("%d,",receivedVector0[j]);
            } else {
                printf("%d ",receivedVector0[j]);
            }
        }
        printf("1:");
        for (int j = 0; j < noWorkers; j++) {
            if (j < noWorkers - 1) {
                printf("%d,",ranks[j]);
            } else {
                printf("%d ",ranks[j]);
            }
        }
        printf("2:");
        for (int j = 0; j < received2; j++) {
            if (j < received2 - 1) {
                printf("%d,",receivedVector2[j]);
            } else {
                printf("%d ",receivedVector2[j]);
            }
        }
        printf("\n");
    } else if (rank == 2) {
        FILE* inputFile = fopen("cluster2.txt", "r");
        int *ranks;
        int received0, *receivedVector0;
        int received1, *receivedVector1;
        
        MPI_Status status;

        fscanf(inputFile, "%d", &noWorkers);
        ranks = (int*)calloc(noWorkers, sizeof(int));
        
        for (int i = 0; i < noWorkers; i++)
            fscanf(inputFile, "%d", &ranks[i]);
        
        fclose(inputFile);

        // receive from 0
        MPI_Recv(&received0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        receivedVector0 = (int*)calloc(received0, sizeof(int));
        MPI_Recv(receivedVector0, received0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        
        // receive from 1
        MPI_Recv(&received1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
        receivedVector1 = (int*)calloc(received1, sizeof(int));
        MPI_Recv(receivedVector1, received1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);

        // send to 0
        MPI_Send(&noWorkers, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkers, MPI_INT, 0, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);

        // send to 1
        MPI_Send(&noWorkers, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkers, MPI_INT, 1, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);
        
        // send to children
        for (int i = 0; i < noWorkers; i++) {
            MPI_Send(&rank, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            MPI_Send(&noWorkers, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD); 
            MPI_Send(ranks, noWorkers, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            MPI_Send(&received0, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(receivedVector0, received0, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(&received1, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            MPI_Send(receivedVector1, received1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, ranks[i]);
        }
        printf("2 -> ");
        printf("0:");
        for (int j = 0; j < received0; j++) {
            if (j < received0 - 1) {
                printf("%d,",receivedVector0[j]);
            } else {
                printf("%d ",receivedVector0[j]);
            }
        }
        printf("1:");
        for (int j = 0; j < received1; j++) {
            if (j < received1 - 1) {
                printf("%d,",receivedVector1[j]);
            } else {
                printf("%d ",receivedVector1[j]);
            }
        }
        printf("2:");
        for (int j = 0; j < noWorkers; j++) {
            if (j < noWorkers - 1) {
                printf("%d,",ranks[j]);
            } else {
                printf("%d ",ranks[j]);
            }
        }
        printf("\n");
    } else {
        int leader;
        int received0, *receivedVector0;
        int received1, *receivedVector1;
        int received2, *receivedVector2;

        MPI_Status status;

        MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // receive from leader
        MPI_Recv(&received0, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        receivedVector0 = (int*)calloc(received0, sizeof(int));
        MPI_Recv(receivedVector0, received0, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);

        MPI_Recv(&received1, 1, MPI_INT, leader, 1, MPI_COMM_WORLD, &status);
        receivedVector1 = (int*)calloc(received1, sizeof(int));
        MPI_Recv(receivedVector1, received1, MPI_INT, leader, 1, MPI_COMM_WORLD, &status);

        MPI_Recv(&received2, 1, MPI_INT, leader, 2, MPI_COMM_WORLD, &status);
        receivedVector2 = (int*)calloc(received2, sizeof(int));
        MPI_Recv(receivedVector2, received2, MPI_INT, leader, 2, MPI_COMM_WORLD, &status);
        printf("%d -> ", rank);
        printf("0:");
        for (int j = 0; j < received0; j++) {
            if (j < received0 - 1) {
                printf("%d,",receivedVector0[j]);
            } else {
                printf("%d ",receivedVector0[j]);
            }
        }
        printf("1:");
        for (int j = 0; j < received1; j++) {
            if (j < received1 - 1) {
                printf("%d,",receivedVector1[j]);
            } else {
                printf("%d ",receivedVector1[j]);
            }
        }
        printf("2:");
        for (int j = 0; j < received2; j++) {
            if (j < received2 - 1) {
                printf("%d,",receivedVector2[j]);
            } else {
                printf("%d ",receivedVector2[j]);
            }
        }
        printf("\n");
    }
    

    MPI_Finalize();
}