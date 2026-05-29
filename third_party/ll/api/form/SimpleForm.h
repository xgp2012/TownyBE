#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace ll {
namespace form {

enum class FormCancelReason : int {
    Unknown,
    UserCancelled,
    InvalidSelection,
};

enum class ModalFormSelectedButton : int {
    Upper,
    Lower,
};

template<typename T>
class Form {
public:
    std::string content_;
};

class SimpleForm : public Form<int> {
public:
    SimpleForm(const std::string& title, const std::string& content = "")
        : Form<int>(), content_(content) {}
    void appendButton(const std::string& text) { buttons_.push_back(text); }
    void setContent(const std::string& content) { content_ = content; }
    const std::string& content() const { return content_; }
    template<typename T, typename Callback>
    void sendTo(T& sender, Callback cb) {}
private:
    std::vector<std::string> buttons_;
};

class CustomForm : public Form<std::vector<std::string>> {
public:
    CustomForm(const std::string& title) : Form<std::vector<std::string>>(), title_(title) {}
    void addLabel(const std::string& text) { labels_.push_back(text); }
    void addInput(const std::string& label, const std::string& placeholder = "") {
        inputs_.push_back({label, placeholder});
    }
    void sendTo(auto& sender, auto cb) {}
private:
    std::string title_;
    std::vector<std::string> labels_;
    struct Input { std::string label; std::string placeholder; };
    std::vector<Input> inputs_;
};

class ModalForm : public Form<ModalFormSelectedButton> {
public:
    ModalForm(const std::string& title, const std::string& content,
              const std::string& buttonA, const std::string& buttonB)
        : title_(title), content_(content), buttonA_(buttonA), buttonB_(buttonB) {}
    void sendTo(auto& sender, auto cb) {}
private:
    std::string title_, content_, buttonA_, buttonB_;
};

}
}
