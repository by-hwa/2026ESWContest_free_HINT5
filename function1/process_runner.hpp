#pragma once

#include <string>
#include <vector>

struct ProcessResult
{
    int exit_code = -1;
    std::string output;
};

// argv를 셸 없이 실행한다. 모델 응답/사용자 질문이 명령으로 해석되지 않도록 한다.
ProcessResult runProcess(const std::vector<std::string>& argv);
std::string trimOutput(std::string text);
