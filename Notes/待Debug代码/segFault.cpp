#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>

using namespace std;

bool compare(const string &lhs, const string &rhs) {
    string s = lhs + rhs;
    int i = 0, j = lhs.size();
    while (s[i] == s[j]) {
        i++;
        if (i == s.size())
            return true;
        j++;
        if (j == s.size())
            j = 0;
    }
    return s[i] < s[j];
}

class Solution {
public:
    string minNumber(const vector<int> &nums) {
        vector<string> vs;
        for_each(nums.begin(), nums.end(), [&](int val) { vs.emplace_back(to_string(val)); });
        sort(vs.begin(), vs.end(), compare);
        return accumulate(vs.begin(), vs.end(), ""s);
    }
};

int main() {
	cout << Solution().minNumber({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}) << endl;
}

// leetcode https://leetcode.cn/problems/ba-shu-zu-pai-cheng-zui-xiao-de-shu-lcof/

// 在sort过程中字符串地址异常突发段错误