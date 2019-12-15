#ifndef __monitor_h
#define __monitor_h

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#endif

#include <time.h>
#include <iostream>
#include <fstream>
#include <thread>
using namespace std::chrono_literals;

#define BUFFER "buffer.txt"
#define LOGS "logs.txt"

int uniform_distribution(int a, int b) {
    return a + rand() % (b - a + 1);
}

std::string current_time() {
	time_t current_time;
	time(& current_time);
	std::string date = ctime(& current_time);
	return date.substr(8, 2) + " " + date.substr(4, 3) + " " + date.substr(20, 4) + " " + date.substr(11, 8);
}

class Semaphore {
public:

  Semaphore( int value ) {
#ifdef _WIN32
	sem = CreateSemaphore( NULL, value, 1, NULL );
#else
     if( sem_init( & sem, 0, value ) != 0 )
       throw "sem_init: failed";
#endif
  }
  ~Semaphore() {
#ifdef _WIN32
	CloseHandle( sem );
#else
	  sem_destroy( & sem );
#endif
  }

  void p() {
#ifdef _WIN32
	  WaitForSingleObject( sem, INFINITE );
#else
     if( sem_wait( & sem ) != 0 )
       throw "sem_wait: failed";
#endif
  }

  void v() {
#ifdef _WIN32
	  ReleaseSemaphore( sem, 1, NULL );
#else
     if( sem_post( & sem ) != 0 )
       throw "sem_post: failed";
#endif
  }


private:

#ifdef _WIN32
	HANDLE sem;
#else
	sem_t sem;
#endif
};

class Condition {
  friend class Monitor;

public:
	Condition() : w( 0 ) {
		waitingCount = 0;
	}

	void wait() {
		w.p();
	}

	bool signal() {
		if(waitingCount) {
			-- waitingCount;
			w.v();
			return true;
		}//if
		else
			return false;
	}

private:
	Semaphore w;
	int waitingCount; //liczba oczekujacych watkow
};


class Monitor
{
public:
	Monitor() : s(1) {}
    Monitor(int magazine_count, int magazine_size): s(1), magazine_count(magazine_count), magazine_size(magazine_size) {}

	void enter() {
		s.p();
	}

	void leave() {
		s.v();
	}

	void wait(Condition & cond) {
		++ cond.waitingCount;
		leave();
		cond.wait();
	}

	void signal(Condition & cond) {
		if(cond.signal())
			enter();
	}

    void enter_consumer(int a, int b, int id, std::string filename) {
        enter();
        std::this_thread::sleep_for(1s);

        int consume_count = uniform_distribution(a, b);
        std::ofstream log, buffer, logs;

        log.open(filename, std::ios::app);
        log << "[" << current_time() << "] Consumer " << id << " wants to take " << consume_count << " products." << std::endl;
        log.close();
        logs.open(LOGS, std::ios::app);
        logs << "[" << current_time() << "] Consumer " << id << " wants to take " << consume_count << " products." << std::endl;
        logs.close();

        while (magazine_count < consume_count) {
            log.open(filename, std::ios::app);
            log << "[" << current_time() << "] Consumer " << id << " waits with " << consume_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
            log.close();
            logs.open(LOGS, std::ios::app);
            logs << "[" << current_time() << "] Consumer " << id << " waits with " << consume_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
            logs.close();

            wait(empty);

            log.open(filename, std::ios::app);
            log << "[" << current_time() << "] Consumer " << id << " wants to take " << consume_count << " products." << std::endl;
            log.close();
            logs.open(LOGS, std::ios::app);
            logs << "[" << current_time() << "] Consumer " << id << " wants to take " << consume_count << " products." << std::endl;
            logs.close();
        }
        magazine_count -= consume_count;
        log.open(filename, std::ios::app);
        log << "[" << current_time() << "] Consumer " << id << " take " << consume_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
        log.close();
        logs.open(LOGS, std::ios::app);
        logs << "[" << current_time() << "] Consumer " << id << " take " << consume_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
        logs.close();
        buffer.open(BUFFER);
        buffer << magazine_count;
        buffer.close();
        signal(full);

        leave();
    }

    void enter_producer(int c, int d, int id, std::string filename) {
        enter();
        std::this_thread::sleep_for(1s);

        int produce_count = uniform_distribution(c, d);
        std::ofstream log, buffer, logs;

        log.open(filename, std::ios::app);
        log << "[" << current_time() << "] Producer " << id << " wants to add " << produce_count << " products." << std::endl;
        log.close();
        logs.open(LOGS, std::ios::app);
        logs << "[" << current_time() << "] Producer " << id << " wants to add " << produce_count << " products." << std::endl;
        logs.close();

        while (magazine_size - magazine_count < produce_count) {
            log.open(filename, std::ios::app);
            log << "[" << current_time() << "] Producer " << id << " waits with " << produce_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
            log.close();
            logs.open(LOGS, std::ios::app);
            logs << "[" << current_time() << "] Producer " << id << " waits with " << produce_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
            logs.close();

            wait(full);

            log.open(filename, std::ios::app);
            log << "[" << current_time() << "] Producer " << id << " wants to add " << produce_count << " products." << std::endl;
            log.close();
            logs.open(LOGS, std::ios::app);
            logs << "[" << current_time() << "] Producer " << id << " wants to add " << produce_count << " products." << std::endl;
            logs.close();
        }

        magazine_count += produce_count;

        log.open(filename, std::ios::app);
        log << "[" << current_time() << "] Producer " << id << " add " << produce_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
        log.close();
        logs.open(LOGS, std::ios::app);
        logs << "[" << current_time() << "] Producer " << id << " add " << produce_count << " products. Magazine: " << magazine_count << "/" << magazine_size << std::endl;
        logs.close();
        buffer.open(BUFFER);
        buffer << magazine_count;
        buffer.close();

        signal(empty);

        leave();
    }


private:
	Semaphore s;
    Condition full, empty;
    int magazine_count, magazine_size;
};

#endif
