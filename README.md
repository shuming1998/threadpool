# CPP_Thread_Pooling
C++ thread pooling (SHARED library after building)

how to use:

class YourTask : public Task {
public:
    void run() override;
};

int main() {
    ThreadPool pool;
    pool.start();
    pool.submitTask(std::make_shared<YourTask>());
    return 0;
}


