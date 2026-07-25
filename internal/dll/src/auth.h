#pragma once
#include <string>
#include <cstdint>

std::string GetHWID();
bool VerifyKey(const std::string& key);
bool CheckAuth();
bool PromptForKey();
