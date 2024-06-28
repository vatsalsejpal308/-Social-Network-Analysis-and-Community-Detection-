#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <stack>
#include "user.cpp"

class Graph {
private:
    unordered_map<int , User*> users;
    unordered_map<int , unordered_set<int>> adjacencyList;

    map<pair<int , int> , int> calculateEdgeBetweenness(unordered_map<int , unordered_set<int>>& adjTemp) {
        map<pair<int , int> , int> edgeBetweenness;
        for(auto& user : users) {
            int start = user.first;

            unordered_map<int , int> distance;
            unordered_map<int , vector<int>> predecessors;
            unordered_map<int , int> numberShortestPaths;
            queue<int> q;
            stack<int> s;

            for(auto& user0 : users) {
                distance[user0.first] = -1;
                numberShortestPaths[user0.first] = 0;
            }

            distance[start] = 0;
            numberShortestPaths[start] = 1;
            q.push(start);

            while(!q.empty()) {
                int node = q.front(); q.pop();
                s.push(node);

                for(int neighbor : adjTemp[node]) {
                    if(distance[neighbor] == -1) {
                        distance[neighbor] = distance[node] + 1;
                    }

                    if(distance[neighbor] == distance[node] + 1) {
                        numberShortestPaths[neighbor] += numberShortestPaths[node];
                        predecessors[neighbor].push_back(node);
                    }
                }
            }

            unordered_map<int , double> dependency;
            for(auto& user0 : users) {
                dependency[user0.first] = 0.0;
            }

            while(!s.empty()) {
                int node = s.top(); s.pop();

                for(int predecessor : predecessors[node]) {
                    double ratio = (double)numberShortestPaths[predecessor] / numberShortestPaths[node];
                    double delta = ratio * (1.0 + dependency[node]);

                    pair<int , int> edge = minmax(node , predecessor);
                    edgeBetweenness[edge] += delta;
                    dependency[predecessor] += delta;
                }
            }
        }

        return edgeBetweenness;
    }

public:
    string getName(int id) {
        if(users.find(id) != users.end()) {
            return users[id] -> name;
        }
        return "";
    }

    void addUser(string& name , int id) {
        if(users.find(id) == users.end()) {
            User* user = new User(name , id);
            users[id] = user;
        }
    }

    void removeUser(int id) {
        if(users.find(id) != users.end()) {
            adjacencyList.erase(id);

            for(auto& pair : adjacencyList) {
                pair.second.erase(id);
            }

            delete users[id];
            users.erase(id);
        }
    }

    void addConnection(int id1 , int id2) {
        if(users.find(id1) != users.end() && users.find(id2) != users.end()) {
            adjacencyList[id1].insert(id2);
            adjacencyList[id2].insert(id1);
        }
    }

    void removeConnection(int id1 , int id2) {
        if(adjacencyList.find(id1) != adjacencyList.end()) {
            adjacencyList[id1].erase(id2);
        }

        if(adjacencyList.find(id2) != adjacencyList.end()) {
            adjacencyList[id2].erase(id1);
        }
    }

    vector<int> shortestPath(int startId , int endId) {
        if(users.find(startId) == users.end() || users.find(endId) == users.end()) return {};

        unordered_map<int , int> distance;
        unordered_map<int , int> parent;
        unordered_set<int> visited;
        queue<int> q;

        parent[startId] = -1;
        distance[startId] = 0;
        q.push(startId);
        visited.insert(startId);

        while(!q.empty()) {
            int node = q.front(); q.pop();

            for(int neighbor : adjacencyList[node]) {
                if(visited.find(neighbor) == visited.end()) {
                    distance[neighbor] = distance[node] + 1;
                    parent[neighbor] = node;

                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }

        int node = endId;
        vector<int> path;

        while(node != -1) {
            path.push_back(node);
            node = parent[node];
        }

        if(path.back() != startId) return {};

        reverse(path.begin() , path.end());
        return path;
    }

    vector<int> suggestFriends(int id) {
        unordered_map<int , int> mutualFriendsCount;
        for(int friendId : adjacencyList[id]) {
            for(int mutualFriendId : adjacencyList[friendId]) {
                if(mutualFriendId != id && adjacencyList[id].find(mutualFriendId) == adjacencyList[id].end()) {
                    mutualFriendsCount[mutualFriendId]++;
                }
            }
        }

        vector<pair<int , int>> mutualFriends(mutualFriendsCount.begin() , mutualFriendsCount.end());
        sort(mutualFriends.begin() , mutualFriends.end() , [](const pair<int , int>& p1 , const pair<int , int>& p2) {
            return p1.second > p2.second;
        });

        vector<int> suggestions;
        for(pair<int , int>& mutualFriend : mutualFriends) {
            suggestions.push_back(mutualFriend.first);
        }

        return suggestions;
    }

    vector<unordered_set<int>> detectCommunities() {
        vector<unordered_set<int>> communities;
        unordered_map<int , unordered_set<int>> adjTemp = adjacencyList;

        while(!adjTemp.empty()) {
            map<pair<int , int> , int> edgeBetweenness = calculateEdgeBetweenness(adjTemp);

            auto maxEdge = max_element(edgeBetweenness.begin() , edgeBetweenness.end() , [](const auto& a , const auto& b) {
                return a.second < b.second;
            });

            int id1 = maxEdge -> first.first , id2 = maxEdge -> first.second;
            adjTemp[id1].erase(id2);
            adjTemp[id2].erase(id1);

            unordered_set<int> visited;
            for(auto& user : users) {
                int node = user.first;
                if(visited.find(node) == visited.end()) {
                    unordered_set<int> community;
                    queue<int> q;

                    q.push(node);
                    visited.insert(node);

                    while(!q.empty()) {
                        int cur = q.front(); q.pop();

                        community.insert(cur);

                        for(int neighbor : adjTemp[cur]) {
                            if(visited.find(neighbor) == visited.end()) {
                                visited.insert(neighbor);
                                q.push(neighbor);
                            }
                        }
                    }
                    communities.push_back(community);
                }
            }
        }
        return communities;
    }
};