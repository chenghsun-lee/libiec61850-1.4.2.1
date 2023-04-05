/*
 * sv_subscriber_example.c
 *
 * Example program for Sampled Values (SV) subscriber
 *
 */

#include "hal_thread.h"
#include <signal.h>
#include <stdio.h>
#include "sv_subscriber.h"

static bool running = true;


void sigint_handler(int signalId)
{
    running = 0;
}

/* Callback handler for received SV messages */
static void
svUpdateListener(SVSubscriber subscriber, void *parameter, SVSubscriber_ASDU asdu)
{
    printf("svUpdateListener called\n");

    const char *svID = SVSubscriber_ASDU_getSvId(asdu);

    if (svID != NULL)
        printf("  svID=(%s)\n", svID);

    printf("  smpCnt: %i\n", SVSubscriber_ASDU_getSmpCnt(asdu));
    // printf("  confRev: %u\n", SVSubscriber_ASDU_getConfRev(asdu));

    /*
     * Access to the data requires a priori knowledge of the data set.
     * For this example we assume a data set consisting of FLOAT32 values.
     * A FLOAT32 value is encoded as 4 bytes. You can find the first FLOAT32
     * value at byte position 0, the second value at byte position 4, the third
     * value at byte position 8, and so on.
     *
     * To prevent damages due configuration, please check the length of the
     * data block of the SV message before accessing the data.
     */

    if (SVSubscriber_ASDU_getDataSize(asdu) >= 8)
    {
        FILE *vol1_report;
        vol1_report = fopen("vol1.csv", "a");
        // printf("   amp1: %f\n", SVSubscriber_ASDU_getFLOAT64(asdu, 0));
        // printf("   amp2: %f\n", SVSubscriber_ASDU_getFLOAT64(asdu, 8));
        // printf("   amp3: %f\n", SVSubscriber_ASDU_getFLOAT64(asdu, 16));
        // printf("   amp4: %f\n", SVSubscriber_ASDU_getFLOAT64(asdu, 24));

        printf("   vol1: %d\n", SVSubscriber_ASDU_getINT32(asdu, 32));
        fprintf(vol1_report,"%d\n",SVSubscriber_ASDU_getINT32(asdu, 32));
        //fprintf(vol1_report,"%i\n",SVSubscriber_ASDU_getSmpCnt(asdu));
        //printf("   vol2: %d\n", SVSubscriber_ASDU_getINT64(asdu, 40));
        // printf("   vol3: %f\n", SVSubscriber_ASDU_getFLOAT64(asdu, 48));
        // printf("   vol4: %f\n", SVSubscriber_ASDU_getFLOAT64(asdu, 56));
        // printf("   vol4: %f\n", SVSubscriber_ASDU_getFLOAT64(asdu, 64));

        fclose(vol1_report);
    }
}

int main(int argc, char **argv)
{
    FILE *vol1_report;
    vol1_report = fopen("vol1.csv", "w");
    fclose(vol1_report);

    SVReceiver receiver = SVReceiver_create();

    if (argc > 1)
    {
        SVReceiver_setInterfaceId(receiver, argv[1]);
        printf("Set interface id: %s\n", argv[1]);
    }
    else
    {
        printf("Using interface eth0\n");
        SVReceiver_setInterfaceId(receiver, "eth0");
    }

    /* Create a subscriber listening to SV messages with APPID 4000h */
    SVSubscriber subscriber = SVSubscriber_create(NULL, 0x4000);
    /* Install a callback handler for the subscriber */
    SVSubscriber_setListener(subscriber, svUpdateListener, NULL);
    /* Connect the subscriber to the receiver */
    SVReceiver_addSubscriber(receiver, subscriber);
    /* Start listening to SV messages - starts a new receiver background thread */
    SVReceiver_start(receiver);
    if (SVReceiver_isRunning(receiver))
    {
        signal(SIGINT, sigint_handler);

        while (running)
            Thread_sleep(0);

        /* Stop listening to SV messages */
        SVReceiver_stop(receiver);
    }
    else
    {
        printf("Failed to start SV subscriber. Reason can be that the Ethernet interface doesn't exist or root permission are required.\n");
    }

    /* Cleanup and free resources */
    SVReceiver_destroy(receiver);
}
