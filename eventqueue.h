#include <vector>
#include <algorithm>
#include <functional>
#include <mutex>
#include <iostream>

class Channel {
    template <typename T>
    class InternalChannel;
public:


    template <typename tMessage, typename tHandler>
    static void add(tHandler* handler) {
        // typically, the handler type is derived while the message type would be explicit
        // e.g. Channel<MyEvent>::add(this);
        InternalChannel<tMessage>::instance().add(handler); //forward to the appropriate queue
    }

    template <typename tMessage, typename tHandler>
    static void remove(tHandler* handler) {
        InternalChannel<tMessage>::instance().remove(handler);
    }

    template <typename tMessage>
    static void broadcast(const tMessage& message) {
        // usually no need to be explicit, the message type can be derived at compiletime
        InternalChannel<tMessage>::instance().broadcast(message);
    }

private:
    template <typename tMessage>
    class InternalChannel {
    public:
        typedef std::function<void(const tMessage&)> Handler;

        static InternalChannel& instance() {
            static InternalChannel result;
            return result;
        }

        template <typename tHandler>
        void add(tHandler* handler) {
            std::lock_guard<std::mutex> lock(mMutex);

            mHandlers.push_back([handler](const tMessage& msg) { (*handler)(msg); });
            mOriginalPtrs.push_back(handler);
        }

        template <typename tHandler>
        void remove(tHandler* handler) {
            std::lock_guard<std::mutex> lock(mMutex);

            auto it = std::find(mOriginalPtrs.begin(), mOriginalPtrs.end(), handler);
            if (it == mOriginalPtrs.end())
                throw std::runtime_error("Tried to remove a handler that was not in the handler list");

            auto idx = it - mOriginalPtrs.begin();

            mHandlers.erase(mHandlers.begin() + idx);
            mOriginalPtrs.erase(it);
        }

        void broadcast(const tMessage& msg) {
            std::vector<Handler> localQueue(mHandlers.size());

            {
                std::lock_guard<std::mutex> lock(mMutex);
                localQueue = mHandlers;
            }

            for (auto& handler : localQueue)
                handler(msg);
        }

    private:
        std::mutex mMutex;
        std::vector<Handler> mHandlers;
        std::vector<void*> mOriginalPtrs;
    };
};

struct MyEvent { int value; };
struct MyHandler {
    void operator()(const MyEvent& evt) {
        std::cout << "Evt: " << evt.value << "\n";
    }
};

struct MyIntHandler {
    void operator()(const int& i) {
        std::cout << "[int]: " << i << "\n";
    }
};

int main1() {
    MyEvent a{ 123 };
    MyHandler b, c;
    MyIntHandler d, e, f;

    Channel::add<MyEvent>(&b);
    Channel::add<MyEvent>(&c);

    Channel::broadcast(a);
    Channel::broadcast(MyEvent{ 456 });
    Channel::broadcast(a);

    Channel::remove<MyEvent>(&b);
    Channel::broadcast(MyEvent{ 789 });

    Channel::broadcast(111);

    Channel::add<int>(&d);
    Channel::add<int>(&e);
    Channel::add<int>(&f);

    Channel::broadcast(222);

    Channel::remove<int>(&e);

    Channel::broadcast(333);
}