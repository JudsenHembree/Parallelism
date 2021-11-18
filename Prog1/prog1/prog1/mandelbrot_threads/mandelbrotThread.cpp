#include <stdio.h>
#include <algorithm>
#include <pthread.h>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


static inline int mandel(float c_re, float c_im, int count)
{
    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < count; ++i) {

        if (z_re * z_re + z_im * z_im > 4.f)
            break;

        float new_re = z_re*z_re - z_im*z_im;
        float new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }
    return i;
}

//
// workerThreadStart --
//
// Thread entrypoint.
void* workerThreadStart(void* threadArgs) {

    double startTime = CycleTimer::currentSeconds();
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    // TODO: Implement worker thread here.

    //printf("Hello world from thread %d\n", args->threadId);



    //int startRow = (args->height/args->numThreads)*args->threadId;
    //try setting it up for width
    //then try interleaving
    
    //int startRow = (args->height/args->numThreads)*args->threadId;
    //int startRow = args->threadId;

    float dx = (args->x1 - args->x0) / args->width;
    float dy = (args->y1 - args->y0) / args->height;

    //int endRow = startRow + args->height/args->numThreads;
    //int endRow = args->numThreads*(args->height/args->numThreads)+args->threadId;
    //int j = startRow;

    //for (int j = startRow; j < endRow; j=j+args->numThreads) {
    int j = args->threadId;
    while(j<args->height){
        for (int i = 0; i < args->width; ++i) {
            float x = args->x0 + i * dx;
            float y = args->y0 + j * dy;

            int index = (j * args->width + i);
            args->output[index] = mandel(x, y, args->maxIterations);
        }
        j=j+args->numThreads;
    }



    double endTime=CycleTimer::currentSeconds();
    printf("thread %d took %f seconds!\n", args->threadId, endTime-startTime);
    return NULL;
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    const static int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    pthread_t workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    for (int i=0; i<numThreads; i++) {
        // TODO: Set thread arguments here.
        // numThreads  x0    y0   x1   y1    width    height    maxIterations    output);
        args[i].threadId = i;
        args[i].numThreads= numThreads;
        args[i].x0= x0;
        args[i].y0= y0;
        args[i].x1= x1;
        args[i].y1= y1;
        args[i].width= width;
        args[i].height= height;
        args[i].maxIterations= maxIterations;

        args[i].output=output;
    }

    // Fire up the worker threads.  Note that numThreads-1 pthreads
    // are created and the main app thread is used as a worker as
    // well.

    for (int i=1; i<numThreads; i++)
        pthread_create(&workers[i], NULL, workerThreadStart, &args[i]);

    workerThreadStart(&args[0]);

    // wait for worker threads to complete
    for (int i=1; i<numThreads; i++)
        pthread_join(workers[i], NULL);
}
