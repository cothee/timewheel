# timewheel
a simple timewheel, which is a thread-safe version. Users can use Add(Event) in
one thread, Step() in another thread, and Expired() in another new thread; and
Users can use Add(Event) & Expired() in multiple threads, but use Step() just in
one thread.
