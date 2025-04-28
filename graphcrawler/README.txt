Graph Crawler - BFS Traversal

This program crawls the Hollywood graph starting from a given actor up to a specified depth using Breadth-First Search (BFS).

How to build:
$ make

How to run:
$ ./graph_crawler "Tom_Hanks" 2

Requirements:
- libcurl
- rapidjson

Example:
$ ./graph_crawler "Tom_Hanks" 2
Where node name is Tom_Hanks and dist is 2

Example output for Tom_Hanks dist 2:
Tom_Hanks (depth 0)
Forrest_Gump (depth 1)
Saving_Private_Ryan (depth 1)
Cast_Away (depth 1)
Toy_Story (depth 1)
The_Terminal (depth 1)
Apollo_13 (depth 1)
Big (depth 1)
Catch_Me_If_You_Can (depth 1)
Philadelphia (depth 1)
...
Forrest_Gump (depth 1)
Robert_Zemeckis (depth 2)
Robin_Wright (depth 2)
Gary_Sinise (depth 2)
Eric_Roth (depth 2)
Winston_Groom (depth 2)
...
Saving_Private_Ryan (depth 1)
Steven_Spielberg (depth 2)
Matt_Damon (depth 2)
Vin_Diesel (depth 2)
Tom_Sizemore (depth 2)
Edward_Burns (depth 2)
...
Cast_Away (depth 1)
Helen_Hunt (depth 2)
Robert_Zemeckis (already visited)
...
Traversal completed in 1.57 seconds.

