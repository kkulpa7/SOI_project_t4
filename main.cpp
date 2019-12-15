#include "monitor.h"
using namespace std;

struct pc {
    int min, max, id;
    Monitor *magazine;
};

void *pthread_producer(void *args) {
    pc *arg = (pc *) args;
    string filename = "producer" + to_string(arg->id) + ".txt";
    ofstream log, logs;
    log.open(filename);
    log << "[" << current_time() << "] Producer " << arg->id << " starts. " << endl;
    log.close();
    logs.open(LOGS, ios::app);
    logs << "[" << current_time() << "] Producer " << arg->id << " starts. " << endl;
    logs.close();
    while(1) {
        this_thread::sleep_for(1s);
        arg->magazine->enter_producer(arg->min, arg->max, arg->id, filename);
    }
}

void *pthread_consumer(void *args) {
    pc *arg = (pc *) args;
    string filename = "consumer" + to_string(arg->id) + ".txt";
    ofstream log, logs;
    log.open(filename);
    log << "[" << current_time() << "] Consumer " << arg->id << " starts. " << endl;
    log.close();
    logs.open(LOGS, ios::app);
    logs << "[" << current_time() << "] Consumer " << arg->id << " starts. " << endl;
    logs.close();
    while(1) {
        this_thread::sleep_for(1s);
        arg->magazine->enter_consumer(arg->min, arg->max, arg->id, filename);
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    ofstream buffer, logs;
    buffer.open(BUFFER);
    buffer << "0";
    buffer.close();
    logs.open(LOGS);
    logs.close();

    if(argc < 8) {
        cout << "Not enough arguments!" << endl;
        return 1;
    }
    int magazine_size = atoi(argv[1]);
    int producers_number = atoi(argv[2]);
    int producers_min = atoi(argv[3]);
    int producers_max = atoi(argv[4]);
    int consumers_number = atoi(argv[5]);
    int consumers_min = atoi(argv[6]);
    int consumers_max = atoi(argv[7]);

    Monitor magazine(0, magazine_size);
    pc producers[producers_number], consumers[consumers_number];
    pthread_t pthread_producers[producers_number], pthread_consumers[consumers_number];

    int create_thread;

    for(int i = 0; i < producers_number; i++) {
        producers[i].id = i + 1;
        producers[i].min = producers_min;
        producers[i].max = producers_max;
        producers[i].magazine = &magazine;
        create_thread = pthread_create(&pthread_producers[i], NULL, pthread_producer, (void *)&producers[i]);
        if(create_thread) {
            cout << "Unable to create thread " << create_thread << endl;
            return 1;
        }
    }

    for (int i = 0; i <consumers_number; i++) {
        consumers[i].id = i + 1;
        consumers[i].min = consumers_min;
        consumers[i].max = consumers_max;
        consumers[i].magazine = &magazine;
        create_thread = pthread_create(&pthread_consumers[i], NULL, pthread_consumer, (void *)&consumers[i]);
        if(create_thread) {
            cout << "Unable to create thread " << create_thread << endl;
            return 1;
        }
    }

    for(int i = 0; i < producers_number; i++)
        pthread_join(pthread_producers[i], NULL);
    for (int i = 0; i <consumers_number; i++)
        pthread_join(pthread_consumers[i], NULL);

    return 0;
}
