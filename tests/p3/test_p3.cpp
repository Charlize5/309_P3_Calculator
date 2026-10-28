// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

//all includes fair game

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
//^ you got it besty
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}

};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}
std::string tool_name(int i) { return "tool_" + std::to_string(i); } //forgot to define this earlier. 
}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
    CHECK(!eval("1 / (2 - 2)", &v, &err)); //ADDED
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negative
//
//required
TEST(reg_10000_tools){
ToolRegistry r; //default
Tool* first_ptr = nullptr; //check the first tool added
for (int i = 0; i < 10000; ++i) {//debug, this is supposed to be 10000 not 1000

    auto t = std::make_unique<NamedTool>(tool_name(i)); //autofill, check namespace
    if (i == 0) first_ptr = t.get(); //remember the first tool added
    REQUIRE(r.add(std::move(t)));
    CHECK(r.load_factor() <= 1.0); //check the load factor
 
}
CHECK_EQ(r.size(), std::size_t(10000)); //check the size
CHECK(r.find("tool_0") == first_ptr); //check the first tool added

for (int i = 0; i < 10000; ++i) {
    CHECK(r.find(tool_name(i)) != nullptr); }//check that the tool is found
    CHECK(r.find("tool_10000") == nullptr); //check that the tool is found
    CHECK(r.max_probe_length() <=64); //check bound

    std::printf("   buckets: %zu, load factor: %.3f, longest chain: %zu\n",
                r.bucket_count(), r.load_factor(), r.max_probe_length());
}

//6.1
TEST(reg_null_rejected) {
ToolRegistry r; //makes sure that the size remains correct when null is detected
//cant mess with registered tools
    CHECK(!r.add(nullptr)); 
    CHECK_EQ(r.size(), std::size_t{0});
    REQUIRE(r.add(std::make_unique<NamedTool>("a")));
    CHECK(!r.add(std::unique_ptr<Tool>()));
    CHECK_EQ(r.size(), std::size_t{1});        
}

TEST(reg_for_each_visits_each_once) {
    //does the cisit count match the number of tools??
    //
    ToolRegistry r;
    const std::vector<std::string> names = {"read_file", "write_file", "list_dir", "calculator", "echo"};
    for (const auto& n : names) REQUIRE(r.add(std::make_unique<NamedTool>(n)));
    r.add(std::make_unique<NamedTool>("echo"));  // rejected duplicate must not show up twice
//^^!! copies floating everywhere 

    std::vector<std::string> seen;
    r.for_each([&](const Tool& t) { seen.push_back(t.name()); });
    CHECK_EQ(seen.size(), names.size());
    for (const auto& n : names) {
        int count = 0;
        for (const auto& s : seen) if (s == n) ++count;
        CHECK_EQ(count, 1);                      // each name exactly once
    }
}
//////////////////////////////////////////////////////////////////////////////////////
//required:
TEST(reg_alternative_hash) {
    ToolRegistry r; //default
    //add 10000 tools

    for (int i = 0; i <10000; ++i){
        REQUIRE(r.add(std::make_unique<NamedTool>(tool_name(i))));
        //longest chain for +s.size after s at 0
    }

    const std::size_t buckets = r.bucket_count();
    std::vector<int> weak_counts(buckets, 0); //count the number of tools in each bucket
    for (int i = 0; i < 10000; ++i) {
        std::string s = tool_name(i);//name og tool
        std::size_t weak = static_cast<unsigned char>(s[0]) + s.size(); //weak hash
        ++weak_counts[weak % buckets]; //increment the count for the bucket
}
int weak_longest = 0; //find the longest chain for the weak hash
for (int count : weak_counts) {
    if (count > weak_longest) weak_longest = count;
}
std::printf("  at %zu buckets, longest chain: %zu, weak longest chain: %d\n",
            buckets, r.max_probe_length(), weak_longest);
  CHECK(static_cast<std::size_t>(weak_longest) > r.max_probe_length()); //check that the weak hash has a longer chain than the real hash
CHECK(weak_longest > 64); //check bound
}
//requred one workspace per path :( am sad
//literally the same test core with slightly different syntax 
TEST(ws_rejects_parent_escape) {
    TempDir d; //create a temporary directory for the workspace
    Workspace ws(d.path());
    fs::path out;
    std::string reason; 
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason)); //check that the path is rejected
    CHECK(!reason.empty()); 
}
 //clion autofill is so much better
TEST(ws_accepts_plain_relative) {
    TempDir d;
    Workspace ws(d.path()); //
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt"); // 
}
 
TEST(ws_normalizes_dot_and_double_slash) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("./notes//todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt"); //path was normailized? check spec syntax, im not confident in it...
}
 
TEST(ws_dot_is_root) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve(".", &out, &reason)); //check that the path is resolved to the root
    CHECK(out == ws.root());
}
 
TEST(ws_dotdot_that_stays_inside) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("a/../b", &out, &reason));
    CHECK(out == ws.root() / "b");
    REQUIRE(ws.resolve("a/..", &out, &reason)); //requires that the path is in boucnds /../
    CHECK(out == ws.root());
}
 
TEST(ws_accepts_nonexistent_target) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("new/dir/file.txt", &out, &reason)); //resolve a path to a non-existent file
    CHECK(out == ws.root() / "new" / "dir" / "file.txt");
    CHECK(!fs::exists(ws.root() / "new"));              // resolve must not touch the files, you cant be creepin around in there
}
 
TEST(ws_double_dot_inside_a_name_is_fine_lol) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("my..notes.txt", &out, &reason));
    CHECK(out == ws.root() / "my..notes.txt"); //niche but checks for .. 
}
 
TEST(ws_rejects_empty) { //yes, i know empty was supposed to be the first test, but i wanted to do the other ones first
    TempDir d; 
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("", &out, &reason));
    CHECK(!reason.empty());
}
 
TEST(ws_rejects_nul_byte) { //nul char = evil
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    const std::string with_nul = std::string("notes") + '\0' + "/x";   // nuh uh
    CHECK(with_nul.size() == 8);
    CHECK(!ws.resolve(with_nul, &out, &reason));
    CHECK(!reason.empty()); 
}
 
TEST(ws_rejects_absolute) {
    TempDir d; 
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());
    reason.clear();
    // absolute paths not allowed even if they point inside the wokrspae 
    CHECK(!ws.resolve((ws.root() / "todo.txt").string(), &out, &reason));
    CHECK(!reason.empty());
}
 
TEST(ws_rejects_bare_dotdot) {
    TempDir d; 
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("..", &out, &reason)); //..
    CHECK(!reason.empty());
}
 
TEST(ws_rejects_climb_out) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
    reason.clear();
    CHECK(!ws.resolve("a/../../b", &out, &reason)); //climb out of bounds, stop creepin around in my directories
    CHECK(!reason.empty());
}
 
TEST(ws_rejects_climb_out_and_back_in) { 
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    const std::string sneaky = "notes/../../" + ws.root().filename().string() + "/notes/todo.txt";
    CHECK(!ws.resolve(sneaky, &out, &reason)); //I cought you, sneaky.
    CHECK(!reason.empty());
}
 
//6. **Calculator:** every row of the table in §4.6, plus at least two expressions of your own. Use the `eval` helper in the starter's test file, which runs the provided `calc_tokenize` and then your `calc_evaluate`.
//MY CALC TESTS using the eval function at the top
//my divide by zero was added above
TEST(calc_PEMDAS){
    double n = 0;
    std::string err;
    REQUIRE(eval("(2 + 3) * 4", &n, &err));
    CHECK(n == 20);
    REQUIRE(eval("2 * (3 + 4) - 5", &n, &err));
    CHECK(n == 9);
}
TEST(calc_left_to_right){
    double n = 0;
    std::string err; //wrong
    REQUIRE(eval("10 - 4 - 3", &n, &err)); 
    CHECK(n == 3); //make sure n != 9
    REQUIRE(eval("100 / 10 / 5", &n, &err));
    CHECK(n == 2);
}

TEST(calc_doubles){
    double n = 0;
    std::string err;
    REQUIRE(eval("7 / 2", &n, &err));
    CHECK(n == 3.5);
    REQUIRE(eval("3 / 2", &n, &err));
    CHECK(n == 1.5); //ADDED
    REQUIRE(eval("0.25 + 0.25", &n, &err));
    CHECK(n == 0.5); //added
}

TEST(calc_negative){
    double n = 0;
    std::string err;
    REQUIRE(eval("-3 * -3", &n, &err));
    CHECK(n == 9);
}



int main(int argc, char** argv) { return mt::run_all(argc, argv); }
