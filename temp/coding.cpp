#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <vector>

using namespace std;

void compress(deque<int> &dq, string &s) {
    int i = 0;
    for (auto &c : s) {
        if (i & (1 << (c - 'a'))) return;
        else
            i |= (1 << (c - 'a'));
    }
    dq.emplace_back(i);
}

int solution(string &S, vector<int> &C) {
    int index = 0, size = S.size(), ans = 0;
    while (index < size) {
        int cur = index, sum = 0, max = 0;
        while (cur < size && S[cur] == S[index]) {
            sum += C[index];
            max = max < C[index] ? C[index] : max;
            index++;
        }
        ans += sum - max;
    }
    return ans;
}

int minCost(string s, vector<int> &cost) {
    int i = 0, N = s.size(), ans = 0;
    while (i < N) {
        int j = i, sum = 0, mx = 1;
        for (; i < N && s[i] == s[j]; ++i)
            sum += cost[i], mx = max(mx, cost[i]);
        ans += sum - mx;
    }
    return ans;
}

// bool binary_search(int x, vector<int>& v){
//     int low = 0;
//     int high = v.size() - 1;
//     while (low <= high) {
//         int mid = (low + high) / 2;
//         if (v[mid] == x) {
//             return true;
//         } else if (v[mid] < x) {
//             low = mid + 1;
//         } else {
//             high = mid - 1;
//         }
//     }
//     return false;
// }

int solution(vector<int> &A) {
    unordered_set<int> s;
    for (int i = 0; i < A.size(); i++) { s.insert(A[i]); }
    numeric_limits<int>::max();

    for (int i = 1; i <= A.size() + 1; i++) {
        if (s.find(i) != s.end()) return i;
    }
}

int solution(int N, vector<int> &A, vector<int> &B) {
    vector<int> v(N + 1, 0);
    for (size_t i = 0; i < A.size(); i++) {
        v[A[i]]++;
        v[B[i]]++;
    }
    sort(v.begin(), v.end());

    int total = 0;

    for (size_t i = 0; i < v.size(); i++){
        total += v[i] * i;
    }
    return total;
}