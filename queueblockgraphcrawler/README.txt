To compile and run:

1. Run make to compile - or make clean if needed

2. Run ex: ./level_client "Tom Hanks" 4 8 > output_log.txt
           where 4 is depth and 8 is num threads


Tom Hanks at depth 4 with 8 threads: 23879 new nodes discovered, time to crawl was 66.719s
Tom Hanks at depth 4 with 4 threads: 23879 new nodes discovered, time to crawl was 132.484s

Tom Hanks at depth 3 with 8 threads: 5023 new nodes discovered, time to crawl was 10.2659s
Tom Hanks at depth 3 with 4 threads: 5023 new nodes discovered, time to crawl was 20.4105s

The performance at 8 threads is very similar to the last parallel implementation of graph crawler. With the difference usually being less than 1s.
