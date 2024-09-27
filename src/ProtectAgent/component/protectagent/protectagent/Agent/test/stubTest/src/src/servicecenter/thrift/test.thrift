namespace cpp thrifttest

struct Task {
    1:i64 jobID
    2:string info
}

service TaskService 
{
    Task GetTask(1:i64 id)
}


struct Config {
    1:i32 port
    2:string host
}

service MountService 
{
    i64 Mount(1:Config config, 2:string path)
    i64 Unmount(2:string path)
}
