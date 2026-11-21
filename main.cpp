#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Event {
    int id;

    Event(int id) : id(id) {}
};

mutex mtx;
condition_variable cv;

unique_ptr<Event> event;
bool finished = false;

void producer() {
    for (int i = 1; i <= 10; i++) {
        this_thread::sleep_for(chrono::seconds(1));

        unique_lock<mutex> lock(mtx);

        cv.wait(lock, [] {
            return event == nullptr;
        });

        event = make_unique<Event>(i);
        cout << "Отправлено событие " << i << endl;

        lock.unlock();
        cv.notify_one();
    }

    unique_lock<mutex> lock(mtx);

    cv.wait(lock, [] {
        return event == nullptr;
    });

    finished = true;

    lock.unlock();
    cv.notify_one();
}

void consumer() {
    while (true) {
        unique_lock<mutex> lock(mtx);

        cv.wait(lock, [] {
            return event != nullptr || finished;
        });

        if (finished && event == nullptr) {
            break;
        }

        cout << "Обработано событие " << event->id << endl;

        event.reset();

        lock.unlock();
        cv.notify_one();
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    thread t1(producer);
    thread t2(consumer);

    t1.join();
    t2.join();

    return 0;
}
