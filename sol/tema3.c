#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void fatal(char * err_message)
{
    perror(err_message);
    exit(0);
}

void printTopology(int rank, int dimension0, int dimension1, int dimension2,
                                int* buffer0, int* buffer1, int* buffer2){
    int i = 0;
    printf("%d -> ", rank);
        printf("0:");
        for (i = 0; i < dimension0; i++) {
            if (i < dimension0 - 1) {
                printf("%d,",buffer0[i]);
            } else {
                printf("%d ",buffer0[i]);
            }
        }
        printf("1:");
        for (i = 0; i < dimension1; i++) {
            if (i < dimension1 - 1) {
                printf("%d,",buffer1[i]);
            } else {
                printf("%d ",buffer1[i]);
            }
        }
        printf("2:");
        for (i = 0; i < dimension2; i++) {
            if (i < dimension2 - 1) {
                printf("%d,",buffer2[i]);
            } else {
                printf("%d ",buffer2[i]);
            }
        }
        printf("\n");      
    }

int main (int argc, char *argv[]) {
    int numtasks, rank, len;
 
    int dimension, commError;

    int nrSlaves;
    

    sscanf (argv[1],"%d",&dimension);
    sscanf (argv[2],"%d",&commError);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int i,j = 0;

    int leader;
    int *slaves;
    int nrSlaves0, *slavesCluster0;
    int nrSlaves1, *slavesCluster1;
    int nrSlaves2, *slavesCluster2;

    int nrElemChunk, *chunkOfElem;

    int chunkCluster0, chunkCluster1, chunkCluster2;

    int* bufferResult0;
    int* bufferResult1; 
    int* bufferResult2;

    int slaveRation;

    int* final;
    if(!commError){
        if (rank == 0) {
        FILE* inputFile = fopen("cluster0.txt", "r");
        
        // read from cluster
        fscanf(inputFile, "%d", &nrSlaves);
        slaves = (int*)calloc(nrSlaves, sizeof(int));
        if(!slaves){
            fatal("ERROR: Calloc failed!");
        }
        
        for (i = 0; i < nrSlaves; i++)
            fscanf(inputFile, "%d", &slaves[i]);
            
        fclose(inputFile);
        

        // send topology to 1
        MPI_Send(&nrSlaves, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(slaves, nrSlaves, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);
        
        
        // send topology to 2
        MPI_Send(&nrSlaves, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(slaves, nrSlaves, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);


        // receive topology from 1
        MPI_Recv(&nrSlaves1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        slavesCluster1 = (int*)calloc(nrSlaves1, sizeof(int));
        if(!slavesCluster1){
            fatal("ERROR: Calloc failed!");
        }
        MPI_Recv(slavesCluster1, nrSlaves1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // receive topology from 2
        MPI_Recv(&nrSlaves2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        slavesCluster2 = (int*)calloc(nrSlaves2, sizeof(int));
        if(!slavesCluster2){
            fatal("ERROR: Calloc failed!");
        }
        MPI_Recv(slavesCluster2, nrSlaves2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (i = 0; i < nrSlaves; i++) {
            // *insert bici sound* "Eu sunt procesul lider"
            MPI_Send(&rank, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            // send how many slaves has current leader
            MPI_Send(&nrSlaves, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            // send current topology
            MPI_Send(slaves, nrSlaves, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            // send topology to cluster 1
            MPI_Send(&nrSlaves1, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
            MPI_Send(slavesCluster1, nrSlaves1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
            // send topology to cluster 2
            MPI_Send(&nrSlaves2, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
            MPI_Send(slavesCluster2, nrSlaves2, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
            // print message
            printf("M(%d,%d)\n", rank, slaves[i]);
        }
        
        // print topology
        printTopology(rank, nrSlaves, nrSlaves1, nrSlaves2, slaves, slavesCluster1, slavesCluster2);

        // task 2

        // generate vector
        int* task2Buffer = (int*)calloc(dimension, sizeof(int));
        if(!task2Buffer){
            fatal("ERROR: Calloc failed!");
        }
        for (i = 0; i < dimension; i++) {
            task2Buffer[i] = i;
        }
        

        //set nr of task for ecah cluster        
        chunkCluster0 = (int) floor(dimension / (nrSlaves + nrSlaves1 + nrSlaves2)) * nrSlaves; 
        chunkCluster1 = (int) floor(dimension / (nrSlaves + nrSlaves1 + nrSlaves2)) * nrSlaves1; 
        chunkCluster2 = dimension - (chunkCluster0 + chunkCluster1);

        // set task for each slave
        int token = chunkCluster0 / nrSlaves;
        
        // put slaves to work
        for (i = 0; i < nrSlaves; i++) {
            MPI_Send(&token, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            for (j = i * token; j < (i + 1) * token; j++) {
                MPI_Send(&task2Buffer[j], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            }
        }


        // send tasks for cluster 1
        MPI_Send(&chunkCluster1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        for (i = chunkCluster0; i < chunkCluster0 + chunkCluster1; i++) {
            MPI_Send(&task2Buffer[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }
        
        
        // send tasks for cluster 2
        MPI_Send(&chunkCluster2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        for (i = chunkCluster0 + chunkCluster1; i < dimension; i++) {
            MPI_Send(&task2Buffer[i], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        }

        // create final solution
        final = (int*)calloc(dimension, sizeof(int));
        if(!final){
            fatal("ERROR: Calloc failed!");
        }
        int counter = 0;

        // receive computed elem from own slave and add to solution
        bufferResult0 = (int*)calloc(chunkCluster0, sizeof(int));
        if(!bufferResult0){
            fatal("ERROR: Calloc failed!");
        }
        int k = 0;
        for (i = 0; i < nrSlaves; i++) {
            MPI_Recv(&slaveRation, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (j = 0; j < slaveRation; j++) {
                MPI_Recv(&bufferResult0[k], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                k++;
            }
        }
        for (i = 0; i < chunkCluster0; i++) {
            final[counter] = bufferResult0[i];
            counter++;
        }
        free(bufferResult0);

        // receive computed elem from cluster 1 and add to solution
        bufferResult1 = (int*)calloc(chunkCluster1, sizeof(int));
        if(!bufferResult1){
            fatal("ERROR: Calloc failed!");
        }

        MPI_Recv(bufferResult1, chunkCluster1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < chunkCluster1; i++) {
            final[counter] = bufferResult1[i];
            counter++;
        }
        free(bufferResult1);

        // receive computed elem from cluster 2 and add to solution
        bufferResult2 = (int*)calloc(chunkCluster2, sizeof(int));
        if(!bufferResult2){
            fatal("ERROR: Calloc failed!");
        }
        MPI_Recv(bufferResult2, chunkCluster2, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < chunkCluster2; i++) {
            final[counter] = bufferResult2[i];
            counter++;
        }
        free(bufferResult2);
        
        // print final solutiom
        printf("Rezultat: ");
        for (i = 0; i < dimension; i++) {
            printf("%d ", final[i]);
        }
        printf("\n");
        free(final);

        } else if (rank == 1) {
            FILE* inputFile = fopen("cluster1.txt", "r");

            MPI_Status status;

            // read from cluster
            fscanf(inputFile, "%d", &nrSlaves);
            slaves = (int*)calloc(nrSlaves, sizeof(int));
            if(!slaves){
                fatal("ERROR: Calloc failed!");
            }               

            
            for (i = 0; i < nrSlaves; i++)
                fscanf(inputFile, "%d", &slaves[i]);

            fclose(inputFile);

            
            // receive topology from 0
            MPI_Recv(&nrSlaves0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster0 = (int*)calloc(nrSlaves0, sizeof(int));
            if(!slavesCluster0){
                fatal("ERROR: Calloc failed!");
            }     
            MPI_Recv(slavesCluster0, nrSlaves0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // send topology to 0
            MPI_Send(&nrSlaves, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(slaves, nrSlaves, MPI_INT, 0, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);
            

            // send topology to 2
            MPI_Send(&nrSlaves, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
            MPI_Send(slaves, nrSlaves, MPI_INT, 2, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);

            // receive topology from 2
            MPI_Recv(&nrSlaves2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster2 = (int*)calloc(nrSlaves2, sizeof(int));
            if(!slavesCluster2){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster2, nrSlaves2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


            for (i = 0; i < nrSlaves; i++) {
                // *insert bici sound* "Eu sunt procesul lider"
                MPI_Send(&rank, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                // send how many slaves has current leader
                MPI_Send(&nrSlaves, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);    
                // send current topology     
                MPI_Send(slaves, nrSlaves, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                // send topology to cluster 0
                MPI_Send(&nrSlaves0, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                MPI_Send(slavesCluster0, nrSlaves0, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // send topology to cluster 2
                MPI_Send(&nrSlaves2, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
                MPI_Send(slavesCluster2, nrSlaves2, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, slaves[i]);
            }
            
            // print topology
            printTopology(rank, nrSlaves0, nrSlaves, nrSlaves2, slavesCluster0, slaves, slavesCluster2);

            // task 2

            // receive elem to compute from cluster 0
            MPI_Recv(&nrElemChunk, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 

            for (i = 0; i < nrElemChunk; i++) {
                 MPI_Recv(&chunkOfElem[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            
             // set task for each slave
            int token = nrElemChunk / nrSlaves;
            for (i = 0; i < nrSlaves; i++) {
                MPI_Send(&token, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // distribute tasks
                for (j = i * token; j < (i + 1) * token; j++) {
                    MPI_Send(&chunkOfElem[j], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                }
            }
            // create a partial result
            free(chunkOfElem);
            int k = 0;
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 
            for (i = 0; i < nrSlaves; i++) {
                MPI_Recv(&slaveRation, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // add the results to the new vector
                for (j = 0; j < slaveRation; j++) {
                    MPI_Recv(&chunkOfElem[k], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    k++;
                }
            }
        

            // send partial result
            MPI_Send(chunkOfElem, nrElemChunk, MPI_INT, 0, 0, MPI_COMM_WORLD);

        } else if (rank == 2) {

            FILE* inputFile = fopen("cluster2.txt", "r");

            // read from cluster
            fscanf(inputFile, "%d", &nrSlaves);
            slaves = (int*)calloc(nrSlaves, sizeof(int));
            if(!slaves){
                fatal("ERROR: Calloc failed!");
            } 
            
            for (i = 0; i < nrSlaves; i++)
                fscanf(inputFile, "%d", &slaves[i]);
            
            fclose(inputFile);

            // receive topology from 0
            MPI_Recv(&nrSlaves0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster0 = (int*)calloc(nrSlaves0, sizeof(int));
            if(!slavesCluster0){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster0, nrSlaves0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // receive topology from 1
            MPI_Recv(&nrSlaves1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster1 = (int*)calloc(nrSlaves1, sizeof(int));
            if(!slavesCluster1){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster1, nrSlaves1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // send current topology to 0
            MPI_Send(&nrSlaves, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            MPI_Send(slaves, nrSlaves, MPI_INT, 0, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);

            // send current topology to 1
            MPI_Send(&nrSlaves, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
            MPI_Send(slaves, nrSlaves, MPI_INT, 1, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);

            
            // send topologies to children
            for (i = 0; i < nrSlaves; i++) {
            // *insert bici sound* "Eu sunt procesul lider"
                MPI_Send(&rank, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
            // send how many slaves has current leader
                MPI_Send(&nrSlaves, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD); 
                MPI_Send(slaves, nrSlaves, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
                // send cluster 0 topology
                MPI_Send(&nrSlaves0, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                MPI_Send(slavesCluster0, nrSlaves0, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // send cluster 1 topology
                MPI_Send(&nrSlaves1, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                MPI_Send(slavesCluster1, nrSlaves1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, slaves[i]);
            }
            
            // print topology
            printTopology(rank, nrSlaves0, nrSlaves1, nrSlaves, slavesCluster0, slavesCluster1, slaves);

            // task 2

            // receive elem to compute from cluster 0
            MPI_Recv(&nrElemChunk, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 
            for (i = 0; i < nrElemChunk; i++) {
                MPI_Recv(&chunkOfElem[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // set task for slaves
            int token = (int) floor(nrElemChunk / nrSlaves);
            for (i = 0; i < nrSlaves; i++) {
                MPI_Send(&token, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // distribute tasks
                for (j = i * token; j < (i + 1) * token; j++) {
                    MPI_Send(&chunkOfElem[j], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                }
            }

            // create a partial solution
            free(chunkOfElem);
            int k = 0;
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 
            for (i = 0; i < nrSlaves; i++) {
                MPI_Recv(&slaveRation, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (j = 0; j < slaveRation; j++) {
                    // add the results 
                    MPI_Recv(&chunkOfElem[k], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    k++;
                }
            }

            
            // send final results to 0
            MPI_Send(chunkOfElem, nrElemChunk, MPI_INT, 0, 0, MPI_COMM_WORLD);
        
        } else {
            
            // find who is the leader
            MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // receive from leader the whole topology
            MPI_Recv(&nrSlaves0, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster0 = (int*)calloc(nrSlaves0, sizeof(int));
            if(!slavesCluster0){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster0, nrSlaves0, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // get info cluster 1
            MPI_Recv(&nrSlaves1, 1, MPI_INT, leader, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster1 = (int*)calloc(nrSlaves1, sizeof(int));
            if(!slavesCluster1){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster1, nrSlaves1, MPI_INT, leader, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // get info cluster 2
            MPI_Recv(&nrSlaves2, 1, MPI_INT, leader, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster2 = (int*)calloc(nrSlaves2, sizeof(int));
            if(!slavesCluster2){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster2, nrSlaves2, MPI_INT, leader, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            printTopology(rank, nrSlaves0, nrSlaves1, nrSlaves2, slavesCluster0, slavesCluster1, slavesCluster2);
            
            // task 2

            // receive number of elem
            MPI_Recv(&nrElemChunk, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 

            // receive partial solution
            for (i = 0; i < nrElemChunk; i++) {
                MPI_Recv(&chunkOfElem[i], 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // compute elem
            for (i = 0; i < nrElemChunk; i++) {
                chunkOfElem[i] *= 2;
            }

            // tell partial solution lenght
            MPI_Send(&nrElemChunk, 1, MPI_INT, leader, 0, MPI_COMM_WORLD);

            // send partial solution
            for (j = 0; j < nrElemChunk; j++) {
                MPI_Send(&chunkOfElem[j], 1, MPI_INT, leader, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        if (rank == 0) {
        FILE* inputFile = fopen("cluster0.txt", "r");
        
        // read from cluster
        fscanf(inputFile, "%d", &nrSlaves);
        slaves = (int*)calloc(nrSlaves, sizeof(int));
        if(!slaves){
            fatal("ERROR: Calloc failed!");
        } 
        
        for (i = 0; i < nrSlaves; i++)
            fscanf(inputFile, "%d", &slaves[i]);
            
        fclose(inputFile);
                
        // send topology to 2
        MPI_Send(&nrSlaves, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(slaves, nrSlaves, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);


        // receive topology from 1 through 2
        MPI_Recv(&nrSlaves1, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        slavesCluster1 = (int*)calloc(nrSlaves1, sizeof(int));
        if(!slavesCluster1){
            fatal("ERROR: Calloc failed!");
        } 
        MPI_Recv(slavesCluster1, nrSlaves1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        

        // receive topology from 2
        MPI_Recv(&nrSlaves2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        slavesCluster2 = (int*)calloc(nrSlaves2, sizeof(int));
        if(!slavesCluster2){
            fatal("ERROR: Calloc failed!");
        } 
        MPI_Recv(slavesCluster2, nrSlaves2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (i = 0; i < nrSlaves; i++) {
            // * insert sunte de bici * "Eu sunt seful!"
            MPI_Send(&rank, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            // send current cluster slave nr
            MPI_Send(&nrSlaves, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            // send  topology
            MPI_Send(slaves, nrSlaves, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            // send topology for cluster 1
            MPI_Send(&nrSlaves1, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
            MPI_Send(slavesCluster1, nrSlaves1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
            // send topology for cluster 2
            MPI_Send(&nrSlaves2, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
            MPI_Send(slavesCluster2, nrSlaves2, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, slaves[i]);
        }
        
        // print topology
        printTopology(rank, nrSlaves, nrSlaves1, nrSlaves2, slaves, slavesCluster1, slavesCluster2);

        // task 2

        // generate final solution
        int* task2Buffer = (int*)calloc(dimension, sizeof(int));
        if(!task2Buffer){
            fatal("ERROR: Calloc failed!");
        } 
        for (i = 0; i < dimension; i++) {
            task2Buffer[i] = i;
        }
        

        //set nr task for each cluster        
        chunkCluster0 = (int) floor(dimension / (nrSlaves + nrSlaves1 + nrSlaves2)) * nrSlaves; 
        chunkCluster1 = (int) floor(dimension / (nrSlaves + nrSlaves1 + nrSlaves2)) * nrSlaves1; 
        chunkCluster2 = dimension - (chunkCluster0 + chunkCluster1);

        // set task for own slaves
        int token = chunkCluster0 / nrSlaves;
        
        // put slaves to work
        for (i = 0; i < nrSlaves; i++) {
            MPI_Send(&token, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            for (j = i * token; j < (i + 1) * token; j++) {
                MPI_Send(&task2Buffer[j], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
            }
        }


        // send tasks for cluster 1 through 2
        MPI_Send(&chunkCluster1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        for (i = chunkCluster0; i < chunkCluster0 + chunkCluster1; i++) {
                MPI_Send(&task2Buffer[i], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        }
        
        
        // send tasks for cluster 2
        MPI_Send(&chunkCluster2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        for (i = chunkCluster0 + chunkCluster1; i < dimension; i++) {
            MPI_Send(&task2Buffer[i], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        }

        // create final solution
        final = (int*)calloc(dimension, sizeof(int));
        if(!final){
            fatal("ERROR: Calloc failed!");
        } 
        int counter = 0;

        // recive slaves work
        bufferResult0 = (int*)calloc(chunkCluster0, sizeof(int));
        if(!bufferResult0){
            fatal("ERROR: Calloc failed!");
        } 
        int k = 0;
        for (i = 0; i < nrSlaves; i++) {
            MPI_Recv(&slaveRation, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (j = 0; j < slaveRation; j++) {
                MPI_Recv(&bufferResult0[k], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                k++;
            }
        }
        for (i = 0; i < chunkCluster0; i++) {
            final[counter] = bufferResult0[i];
            counter++;
        }
        free(bufferResult0);

        bufferResult1 = (int*)calloc(chunkCluster1, sizeof(int));
        if(!bufferResult1){
            fatal("ERROR: Calloc failed!");
        } 

        MPI_Recv(bufferResult1, chunkCluster1, MPI_INT, 2, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        for (i = 0; i < chunkCluster1; i++) {
            final[counter] = bufferResult1[i];
            counter++;
        }
        free(bufferResult1);

        // receive changes from cluster 2
        bufferResult2 = (int*)calloc(chunkCluster2, sizeof(int));
        if(!bufferResult2){
            fatal("ERROR: Calloc failed!");
        } 
        MPI_Recv(bufferResult2, chunkCluster2, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < chunkCluster2; i++) {
            final[counter] = bufferResult2[i];
            counter++;
        }
        free(bufferResult2);
        
        // print final solution
        printf("Rezultat: ");
        for (i = 0; i < dimension; i++) {
            printf("%d ", final[i]);
        }
        printf("\n");
        free(final);


        } else if (rank == 1) {
            FILE* inputFile = fopen("cluster1.txt", "r");

            MPI_Status status;

            // read from cluster
            fscanf(inputFile, "%d", &nrSlaves);
            slaves = (int*)calloc(nrSlaves, sizeof(int));
            if(!slaves){
                fatal("ERROR: Calloc failed!");
            } 
            
            for (i = 0; i < nrSlaves; i++)
                fscanf(inputFile, "%d", &slaves[i]);

            fclose(inputFile);

            // send topology to 2
            MPI_Send(&nrSlaves, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
            MPI_Send(slaves, nrSlaves, MPI_INT, 2, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);

            // receive topology from 2
            MPI_Recv(&nrSlaves2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster2 = (int*)calloc(nrSlaves2, sizeof(int));
            if(!slavesCluster2){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster2, nrSlaves2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


            // receive topology from 0 through 2
            MPI_Recv(&nrSlaves0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster0 = (int*)calloc(nrSlaves0, sizeof(int));
            if(!slavesCluster0){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster0, nrSlaves0, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            

            for (i = 0; i < nrSlaves; i++) {
                // leader
                MPI_Send(&rank, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                // nr slaves
                MPI_Send(&nrSlaves, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);    
                // slaves  
                MPI_Send(slaves, nrSlaves, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                // info cluster 0
                MPI_Send(&nrSlaves0, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                MPI_Send(slavesCluster0, nrSlaves0, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // info cluster 1
                MPI_Send(&nrSlaves2, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
                MPI_Send(slavesCluster2, nrSlaves2, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, slaves[i]);
            }
            
            //print topology
            printTopology(rank, nrSlaves0, nrSlaves, nrSlaves2, slavesCluster0, slaves, slavesCluster2);

            // task 2

            // receive elem to compute
            MPI_Recv(&nrElemChunk, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 


            for (i = 0; i < nrElemChunk; i++) {
                MPI_Recv(&chunkOfElem[i], nrElemChunk, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            
            
            // set tasks
            int token = nrElemChunk / nrSlaves;
            for (i = 0; i < nrSlaves; i++) {
                MPI_Send(&token, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // distribute tasks
                for (j = i * token; j < (i + 1) * token; j++) {
                    MPI_Send(&chunkOfElem[j], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                }
            }
            // create partial solution
            free(chunkOfElem);
            int k = 0;
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 

            for (i = 0; i < nrSlaves; i++) {
                MPI_Recv(&slaveRation, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // add the results to the new vector
                for (j = 0; j < slaveRation; j++) {
                    MPI_Recv(&chunkOfElem[k], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    k++;
                }
            }
        

            // send final results to cluster 0 through 2
            MPI_Send(&nrElemChunk, 1, MPI_INT, 2, 3, MPI_COMM_WORLD);   
            MPI_Send(chunkOfElem, nrElemChunk, MPI_INT, 2, 3, MPI_COMM_WORLD);
            

        } else if (rank == 2) {

            FILE* inputFile = fopen("cluster2.txt", "r");

            // read input
            fscanf(inputFile, "%d", &nrSlaves);
            slaves = (int*)calloc(nrSlaves, sizeof(int));
            if(!slaves){
                fatal("ERROR: Calloc failed!");
            } 
            
            for (i = 0; i < nrSlaves; i++)
                fscanf(inputFile, "%d", &slaves[i]);
            
            fclose(inputFile);

            // receive topology from 0
            MPI_Recv(&nrSlaves0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster0 = (int*)calloc(nrSlaves0, sizeof(int));
            if(!slavesCluster0){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster0, nrSlaves0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // receive topology from 1
            MPI_Recv(&nrSlaves1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster1 = (int*)calloc(nrSlaves1, sizeof(int));
            if(!slavesCluster1){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster1, nrSlaves1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // send topology to 0
            MPI_Send(&nrSlaves, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            MPI_Send(slaves, nrSlaves, MPI_INT, 0, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);

            // send current topology to 1
            MPI_Send(&nrSlaves, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
            MPI_Send(slaves, nrSlaves, MPI_INT, 1, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);


            // send cluster 0 topology to 1
            MPI_Send(&nrSlaves0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Send(slavesCluster0, nrSlaves0, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);

            // send cluster 1 topology to 0
            MPI_Send(&nrSlaves1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(slavesCluster1, nrSlaves1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);
            
            
            // send topologies to children
            for (i = 0; i < nrSlaves; i++) {
                // leader
                MPI_Send(&rank, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
                // nr slaves
                MPI_Send(&nrSlaves, 1, MPI_INT, slaves[i], 2, MPI_COMM_WORLD); 
                MPI_Send(slaves, nrSlaves, MPI_INT, slaves[i], 2, MPI_COMM_WORLD);
                // info culster 0
                MPI_Send(&nrSlaves0, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                MPI_Send(slavesCluster0, nrSlaves0, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // info cluster 1
                MPI_Send(&nrSlaves1, 1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                MPI_Send(slavesCluster1, nrSlaves1, MPI_INT, slaves[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, slaves[i]);
            }
            
            // start topology
            printTopology(rank, nrSlaves0, nrSlaves1, nrSlaves, slavesCluster0, slavesCluster1, slaves);

            // task 2

            MPI_Recv(&nrElemChunk, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 
            for (i = 0; i < nrElemChunk; i++) {
            MPI_Recv(&chunkOfElem[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            MPI_Send(&nrElemChunk, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            for (i = 0; i < nrElemChunk; i++) {
                MPI_Send(&chunkOfElem[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            }
            

            MPI_Recv(&nrElemChunk, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 
            for (i = 0; i < nrElemChunk; i++) {
                MPI_Recv(&chunkOfElem[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // set tasks
            int token = (int) floor(nrElemChunk / nrSlaves);
            for (i = 0; i < nrSlaves; i++) {
                MPI_Send(&token, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                // distribute tasks
                for (j = i * token; j < (i + 1) * token; j++) {
                    MPI_Send(&chunkOfElem[j], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD);
                }
            }

            // create partial solution
            free(chunkOfElem);
            int k = 0;
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 
            for (i = 0; i < nrSlaves; i++) {
                MPI_Recv(&slaveRation, 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (j = 0; j < slaveRation; j++) {
                    // add the results to the new vector
                    MPI_Recv(&chunkOfElem[k], 1, MPI_INT, slaves[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    k++;
                }
            }

            // send cluster1 final results to cluster 0
            MPI_Recv(&slaveRation, 1, MPI_INT, 1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int* bufferBonus = (int*)calloc(slaveRation, sizeof(int));
            if(!bufferBonus){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(bufferBonus, slaveRation, MPI_INT, 1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(bufferBonus, slaveRation, MPI_INT, 0, 3, MPI_COMM_WORLD);
            
            
            // send final results to 0
            MPI_Send(chunkOfElem, nrElemChunk, MPI_INT, 0, 0, MPI_COMM_WORLD);
        
        } else {
            
            //set leader
            MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


            // info culster 0
            MPI_Recv(&nrSlaves0, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster0 = (int*)calloc(nrSlaves0, sizeof(int));
            if(!slavesCluster0){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster0, nrSlaves0, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // info culster 1
            MPI_Recv(&nrSlaves1, 1, MPI_INT, leader, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster1 = (int*)calloc(nrSlaves1, sizeof(int));
            if(!slavesCluster1){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster1, nrSlaves1, MPI_INT, leader, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // info culster 2
            MPI_Recv(&nrSlaves2, 1, MPI_INT, leader, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            slavesCluster2 = (int*)calloc(nrSlaves2, sizeof(int));
            if(!slavesCluster2){
                fatal("ERROR: Calloc failed!");
            } 
            MPI_Recv(slavesCluster2, nrSlaves2, MPI_INT, leader, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // print toplogy
            printTopology(rank, nrSlaves0, nrSlaves1, nrSlaves2, slavesCluster0, slavesCluster1, slavesCluster2);
            
            // task 2

            // receive number of elements 
            MPI_Recv(&nrElemChunk, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            chunkOfElem = (int*)calloc(nrElemChunk, sizeof(int));
            if(!chunkOfElem){
                fatal("ERROR: Calloc failed!");
            } 

            // receive elem
            for (i = 0; i < nrElemChunk; i++) {
                MPI_Recv(&chunkOfElem[i], 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // compute elem
            for (i = 0; i < nrElemChunk; i++) {
                chunkOfElem[i] *= 2;
            }

            // send nr compted elem
            MPI_Send(&nrElemChunk, 1, MPI_INT, leader, 0, MPI_COMM_WORLD);

            // send computed elem to leader
            for (j = 0; j < nrElemChunk; j++) {
                MPI_Send(&chunkOfElem[j], 1, MPI_INT, leader, 0, MPI_COMM_WORLD);
            }
        }
    }
    
    
    MPI_Finalize();
}