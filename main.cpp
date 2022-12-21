#include <assert.h>
#include <stdlib.h>

#include "consumer_controller.hpp"
#include "item.hpp"
#include "producer.hpp"
#include "reader.hpp"
#include "ts_queue.hpp"
#include "writer.hpp"

#define READER_QUEUE_SIZE 200
#define WORKER_QUEUE_SIZE 200
#define WRITER_QUEUE_SIZE 4000
#define CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE 20
#define CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE 80
#define CONSUMER_CONTROLLER_CHECK_PERIOD 1000000

int main(int argc, char** argv) {
    assert(argc == 4);

    int n = atoi(argv[1]);
    std::string input_file_name(argv[2]);
    std::string output_file_name(argv[3]);

    // TODO: implements main function
    // Initialize
    TSQueue<Item*>* input_queue = new TSQueue<Item*>(READER_QUEUE_SIZE);
    TSQueue<Item*>* worker_queue = new TSQueue<Item*>(WORKER_QUEUE_SIZE);
    TSQueue<Item*>* output_queue = new TSQueue<Item*>(WRITER_QUEUE_SIZE);
    Transformer* transformer = new Transformer;

    Reader* reader = new Reader(n, input_file_name, input_queue);
    Producer* producers[4];
    for (int i = 0; i < 4; i++) {
        producers[i] = new Producer(input_queue, worker_queue, transformer);
    }
    ConsumerController* consumerController = new ConsumerController(
        worker_queue, output_queue, transformer,
        CONSUMER_CONTROLLER_CHECK_PERIOD,
        WORKER_QUEUE_SIZE * ((double)CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE / 100.0),
        WORKER_QUEUE_SIZE * ((double)CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE / 100.0));
    Writer* writer = new Writer(n, output_file_name, output_queue);

    // Start Threads
    reader->start();
    writer->start();
    for (int i = 0; i < 4; i++) {
        producers[i]->start();
    }
    consumerController->start();

    // Join Threads
    reader->join();
    writer->join();

    // Cancel Threads
    for (int i = 0; i < 4; i++) {
        producers[i]->cancel();
    }
    consumerController->cancel();

    delete input_queue;
    delete worker_queue;
    delete output_queue;
    delete reader;
    for (int i = 0; i < 4; i++) {
        delete producers[i];
    }
    delete consumerController;
    delete writer;

    return 0;
}
