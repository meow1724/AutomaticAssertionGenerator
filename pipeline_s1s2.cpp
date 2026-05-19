#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <thread>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <iterator>
#include <cstdio>
#include <memory>
#include <array>
#include <map>
#include <set>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <cstdlib>
#include <climits>
#include <iomanip>

using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;

// --- Safe JSON Helper Functions ---
string get_str(const json& j, const string& key, const string& def) {
    if (!j.contains(key) || j[key].is_null()) return def;
    if (j[key].is_string()) return j[key].get<string>();
    if (j[key].is_number_integer()) return to_string(j[key].get<int>());
    if (j[key].is_boolean()) return j[key].get<bool>() ? "true" : "false";
    return def;
}

json get_array(const json& j, const string& key) {
    if (j.contains(key) && j[key].is_array()) return j[key];
    return json::array();
}

// --- Text Helper Functions ---
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// --- Terminal Subprocess Executor ---
string exec_cmd(const string& cmd) {
    array<char, 256> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen((cmd + " 2>&1").c_str(), "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string repeat_str(const string& s, int n) {
    string res = "";
    for (int i = 0; i < n; i++) res += s;
    return res;
}

string format_python_list(const vector<string>& vec) {
    string res = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        res += "'" + vec[i] + "'";
        if (i < vec.size() - 1) res += ", ";
    }
    res += "]";
    return res;
}

string join_list(const vector<string>& vec) {
    string res = "";
    for (size_t i = 0; i < vec.size(); ++i) {
        res += vec[i];
        if (i < vec.size() - 1) res += ", ";
    }
    return res;
}

int count_regex(const string& text, const string& pattern) {
    regex re(pattern);
    auto words_begin = sregex_iterator(text.begin(), text.end(), re);
    auto words_end = sregex_iterator();
    return distance(words_begin, words_end);
}

json load_json_file(const string& path) {
    ifstream f(path);
    if (!f.is_open()) throw runtime_error("Cannot open JSON: " + path);
    return json::parse(f);
}

string read_file_trunc(const string& path, size_t max_len) {
    ifstream f(path);
    if (!f.is_open()) return "";
    string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    if (content.size() > max_len) return content.substr(0, max_len);
    return content;
}

string build_uncovered_signals_hint(const json& info_bank) {
    json signals = get_array(info_bank, "signals");
    string hint;
    for (const auto& s : signals) {
        if (get_str(s, "direction", "") != "input")
            hint += "  - " + get_str(s, "rtl_name", "?") + "\n";
    }
    return hint.empty() ? "(none listed)\n" : hint;
}

// Given assertable signals and current vault code, return signals NOT referenced in any vault assert.
static string build_real_uncovered_hint(const json& info_bank, const string& vault_asserts_only) {
    json signals = get_array(info_bank, "signals");
    string hint;
    for (const auto& s : signals) {
        if (get_str(s, "direction", "") == "input") continue;
        string name = get_str(s, "rtl_name", "");
        if (name.empty()) continue;
        try {
            regex r("\\b" + name + "\\b");
            if (!regex_search(vault_asserts_only, r))
                hint += "  - " + name + " (no passing assertion yet)\n";
        } catch (...) { continue; }
    }
    return hint.empty() ? "  (all assertable signals have at least one passing assertion)\n" : hint;
}

// Parse SymbiYosys trace_tb.v: extract the per-cycle PI_<input> = <value> assignments as compact text.
// Produces text like:
//   cycle 0: rst=1'b0 enable=1'b0 selection=1'b0 d_in_0=8'b00000000 d_in_1=8'b00000000
//   cycle 1: ...
static string summarize_trace_tb(const string& path, int max_cycles = 10) {
    ifstream f(path);
    if (!f.is_open()) return "";
    string line;
    // Section: lines that look like "    if (cycle == N) begin" ... "      PI_X <= VAL;" ... "    end"
    regex rx_cycle(R"(if\s*\(\s*cycle\s*==\s*(\d+)\s*\)\s*begin)");
    regex rx_assign(R"(PI_([A-Za-z0-9_]+)\s*<=\s*([^;]+);)");
    regex rx_end(R"(^\s*end\s*$)");

    struct CycleBlock { int cycle; vector<pair<string,string>> assigns; };
    vector<CycleBlock> blocks;
    CycleBlock cur; cur.cycle = -1;
    bool in_block = false;
    while (getline(f, line)) {
        smatch m;
        if (!in_block) {
            if (regex_search(line, m, rx_cycle)) {
                cur = CycleBlock{};
                cur.cycle = stoi(m[1]);
                in_block = true;
            }
        } else {
            if (regex_search(line, m, rx_assign)) {
                cur.assigns.push_back({m[1].str(), trim(m[2].str())});
            } else if (regex_search(line, m, rx_end)) {
                blocks.push_back(cur);
                in_block = false;
                if ((int)blocks.size() >= max_cycles) break;
            }
        }
    }
    if (blocks.empty()) return "";
    string out;
    for (const auto& b : blocks) {
        out += "  cycle " + to_string(b.cycle) + ":";
        for (const auto& kv : b.assigns) out += " " + kv.first + "=" + kv.second;
        out += "\n";
    }
    return out;
}

// Extract signals with their functional descriptions (for design hints injected into Stage 5 prompt).
static string build_design_hints(const json& info_bank) {
    string out;
    string mod = get_str(info_bank, "module_name", "top");
    string clk = get_str(info_bank, "clock", "clk");
    string rst = get_str(info_bank, "reset", "rst");
    string pol = get_str(info_bank, "reset_polarity", "");
    out += "Module: " + mod + "; clock=" + clk + "; reset=" + rst;
    if (!pol.empty()) out += " (" + pol + ")";
    out += "\n";
    string mdesc = get_str(info_bank, "module_description", "");
    if (!mdesc.empty()) out += "Description: " + mdesc + "\n";
    out += "Assertable signals (only these may be on LHS of assert):\n";
    json signals = get_array(info_bank, "signals");
    for (const auto& s : signals) {
        if (get_str(s, "direction", "") == "input") continue;
        string name = get_str(s, "rtl_name", "");
        if (name.empty()) continue;
        out += "  - " + name;
        string desc = get_str(s, "functional_description", "");
        string rb   = get_str(s, "reset_behavior", "");
        if (!desc.empty()) out += " — " + desc;
        if (!rb.empty())   out += " [reset: " + rb + "]";
        out += "\n";
    }
    return out;
}

string truncate_prompt_section(const string& s, size_t max_bytes); // fwd

// Wrap bare `label: assert(...)` lines (from stage4 passed[].original_code) in always @(posedge clk)
// blocks. Needed when concatenating vault asserts at module scope — Yosys requires procedural context.
static string wrap_bare_asserts(const string& code, const string& clk_name) {
    istringstream iss(code);
    string line, out;
    regex rx_bare(R"(^\s*[a-zA-Z0-9_]+\s*:\s*(assert|cover)\s*\()");
    regex rx_always(R"(^\s*always\b)");
    while (getline(iss, line)) {
        string t = trim(line);
        if (t.empty()) { out += line + "\n"; continue; }
        if (regex_search(line, rx_always)) { out += line + "\n"; continue; }
        if (regex_search(line, rx_bare)) {
            // Wrap with f_past_valid so $past() is defined. fmsv_init_cnt in the preamble
            // holds rst active for the first cycles; we don't guard on it here because
            // the vacuity checker runs shallow BMC and would otherwise flag every assert as
            // unreachable.
            out += "always @(posedge " + clk_name + ") if (f_past_valid) " + t + "\n";
        } else {
            out += line + "\n";
        }
    }
    return out;
}

// List assertable signals that have ZERO asserts referencing them in the vault.
// Uses only the vault's assert lines (covers are not correctness, so they don't "cover" a signal here).
static vector<string> compute_uncovered_signals(const json& info_bank, const string& vault_asserts_only) {
    vector<string> out;
    for (const auto& s : get_array(info_bank, "signals")) {
        if (get_str(s, "direction", "") == "input") continue;
        string name = get_str(s, "rtl_name", "");
        if (name.empty()) continue;
        try {
            regex r("\\b" + name + "\\b");
            if (!regex_search(vault_asserts_only, r)) out.push_back(name);
        } catch (...) { continue; }
    }
    return out;
}

// Extract case-statement transition graph for an FSM signal (e.g. pstate).
// Returns text like:
//   state 3'b000: if(selection==0) nstate = (d_in_0[0] ? 3'b001 : 3'b000)
// Best-effort textual summary — falls back to empty string if no case found.
static string extract_fsm_transitions(const string& rtl_code, const string& fsm_sig) {
    // Find "case (pstate)" then capture until matching "endcase".
    regex rx_case("case\\s*\\(\\s*" + fsm_sig + "\\s*\\)");
    smatch m;
    if (!regex_search(rtl_code, m, rx_case)) return "";
    size_t start = m.position(0) + m.length(0);
    size_t end = rtl_code.find("endcase", start);
    if (end == string::npos) return "";
    string block = rtl_code.substr(start, end - start);
    // Compact whitespace so it fits in a prompt.
    string compact;
    for (char c : block) {
        if (c == '\n' || c == '\r' || c == '\t') { if (!compact.empty() && compact.back() != ' ') compact += ' '; }
        else compact += c;
    }
    // Collapse multi-spaces
    regex rx_ws("  +");
    compact = regex_replace(compact, rx_ws, " ");
    if (compact.size() > 1800) compact = compact.substr(0, 1800) + " /*...truncated...*/";
    return compact;
}

// Filter LLM output: keep only assertion lines that reference the target signal on the asserted side.
// Returns a cleaned string containing only lines that mention `sig` within an assert(...) or cover(...).
static string filter_properties_mentioning(const string& code, const string& sig) {
    istringstream iss(code);
    string line, out;
    regex rx_sig("\\b" + sig + "\\b");
    regex rx_prop(R"(\b(assert|cover)\s*\()");
    int kept_depth = 0;    // lines of an accepted always-block that we're still emitting
    bool in_always = false;
    string pending_always;
    int brace_depth = 0;
    (void)rx_prop;
    // Simple pass: emit any line inside an always @ block only if the block contains the target signal.
    // Strategy: buffer always @(...) begin ... end blocks and only emit if buffered text contains sig.
    string buffer;
    int begin_count = 0;
    bool buffering = false;
    while (getline(iss, line)) {
        string t = trim(line);
        if (!buffering) {
            if (t.rfind("always", 0) == 0) {
                buffering = true;
                buffer = line + "\n";
                begin_count = 0;
                // count begin/end on this line too
                if (t.find("begin") != string::npos) begin_count++;
                // Single-line always (no begin) — emit immediately if match
                if (begin_count == 0 && t.find(';') != string::npos) {
                    if (regex_search(buffer, rx_sig)) out += buffer;
                    buffering = false; buffer.clear();
                }
                continue;
            }
            // Stray non-always lines (preamble etc.) — drop silently in focused mode.
            continue;
        }
        buffer += line + "\n";
        if (t.find("begin") != string::npos) begin_count++;
        if (t == "end" || t.rfind("end ", 0) == 0 || t == "endmodule") {
            begin_count--;
            if (begin_count <= 0) {
                if (regex_search(buffer, rx_sig)) out += buffer;
                buffering = false; buffer.clear(); begin_count = 0;
            }
        }
    }
    return out;
}

// Build a tightly-scoped prompt asking the LLM to produce assertions ABOUT ONE SIGNAL.
static string build_focused_prompt(const string& sig,
                                   const json& info_bank,
                                   const string& rtl_code,
                                   int round) {
    string out;
    out += "You are writing formal assertions for a SINGLE target signal in an RTL design.\n\n";
    out += "=== TARGET SIGNAL ===\n";
    // Dump the slice for this signal
    for (const auto& s : get_array(info_bank, "signals")) {
        if (get_str(s, "rtl_name", "") != sig) continue;
        out += "name: " + sig + "\n";
        out += "type: " + get_str(s, "type", "") + "\n";
        out += "width: " + get_str(s, "width", "1") + "\n";
        string desc = get_str(s, "functional_description", "");
        if (!desc.empty()) out += "description: " + desc + "\n";
        string rb = get_str(s, "reset_behavior", "");
        if (!rb.empty()) out += "reset_behavior: " + rb + "\n";
        if (s.contains("dependencies") && s["dependencies"].is_array()) {
            out += "depends_on: ";
            for (const auto& d : s["dependencies"]) if (d.is_string()) out += d.get<string>() + " ";
            out += "\n";
        }
        break;
    }
    out += "\n";

    // If it's an FSM state, inject the transition table
    if (sig == "pstate" || sig == "nstate" || sig.find("state") != string::npos) {
        string fsm = extract_fsm_transitions(rtl_code, sig);
        if (fsm.empty() && sig == "nstate") fsm = extract_fsm_transitions(rtl_code, "pstate");
        if (!fsm.empty()) {
            out += "=== FSM TRANSITION TABLE (from case statement) ===\n";
            out += fsm + "\n\n";
        }
    }

    out += "=== RTL (authoritative) ===\n```verilog\n";
    out += truncate_prompt_section(rtl_code, 12000);
    out += "\n```\n\n";
    out += "=== HARD RULES ===\n";
    out += "1. Every assertion MUST reference `" + sig + "` inside the assert(...) expression.\n";
    out += "2. Do NOT write assertions about other signals — they are out of scope for this round.\n";
    out += "3. Do NOT write cover() statements — only assert().\n";
    out += "4. Wrap each in `always @(posedge clk) begin ... end` with an `if (...)` guard OUTSIDE the assert.\n";
    out += "5. Guards using $past() MUST include `f_past_valid` as a conjunct.\n";
    out += "6. Use == only (not ===). No |->, |=>, ##, $isunknown, property/endproperty.\n";
    out += "7. NEVER put the asserted expression inside its own guard (tautology ban).\n";
    out += "8. Labels must be unique: `a_" + sig + "_<context>_rf" + to_string(round) + "`.\n";
    out += "9. Prefer 3-5 DIVERSE properties: a reset-value property, a transition property,\n";
    out += "   and a relational property tying `" + sig + "` to its drivers.\n\n";
    out += "OUTPUT: raw Verilog only. No markdown, no module, no endmodule, no backticks.\n";
    return out;
}

// Keep Stage 5 Groq requests under payload limits (avoid HTTP 413).
string truncate_prompt_section(const string& s, size_t max_bytes) {
    if (s.size() <= max_bytes) return s;
    size_t cut = max_bytes > 200 ? max_bytes - 200 : max_bytes;
    return s.substr(0, cut) + "\n\n// ... [TRUNCATED " + to_string(s.size() - cut) + " bytes for API size limit]\n";
}

// Rebuild the Stage 5 vault FROM SCRATCH from the current stage4_results.json passed[].
// Bug-fix: previous behavior accumulated passes monotonically across rounds, so an assertion
// that passed in round N but regressed in round N+1 stayed in the vault forever, producing
// prompts where the same label appeared in both VAULT (keep unchanged) and FAILED (fix it).
// If include_covers=false, covers are dropped — they're reachability trivia and just eat prompt.
// Returns number of distinct labels seeded.
int vault_rebuild_from_passed(string& locked_passes, set<string>& vault_labels,
                              const json& passed, bool include_covers = false) {
    locked_passes.clear();
    vault_labels.clear();
    int added = 0;
    regex rx_lbl(R"((\b[a-zA-Z0-9_]+)\s*:\s*(assert|cover)\b)");
    regex rx_is_cover(R"(\bcover\s*\()");
    for (const auto& p : passed) {
        if (!p.is_object()) continue;
        if (!p.contains("original_code") || !p["original_code"].is_string()) continue;
        string code = trim(p["original_code"].get<string>());
        if (code.empty()) continue;
        if (!include_covers && regex_search(code, rx_is_cover)) continue;
        string lbl = get_str(p, "label", "");
        if (lbl.empty()) {
            smatch m;
            if (regex_search(code, m, rx_lbl)) lbl = m[1];
            else continue;
        }
        if (vault_labels.count(lbl)) continue;
        vault_labels.insert(lbl);
        locked_passes += code + "\n";
        added++;
    }
    return added;
}

static string join_set_labels(const set<string>& labels) {
    string s;
    for (const auto& lbl : labels) s += lbl + "\n";
    return s;
}

int count_substring(const string& text, const string& sub) {
    int count = 0;
    size_t pos = 0;
    while ((pos = text.find(sub, pos)) != string::npos) {
        count++;
        pos += sub.length();
    }
    return count;
}

string clean_llm(const string& resp) {
    string r = trim(resp);
    if (r.rfind("```", 0) == 0) {
        size_t first_newline = r.find('\n');
        if (first_newline != string::npos) r = r.substr(first_newline + 1);
        r = trim(r);
        if (r.length() >= 3 && r.substr(r.length() - 3) == "```") {
            r = r.substr(0, r.length() - 3);
        }
    }
    return trim(r);
}

// Strip SV-only constructs Yosys can't handle. Minimal, lossless: remove markdown leaks,
// module blocks, property/endproperty, assert property, |->/|=>/##, disable iff, etc.
string clean_assertions(const string& code) {
    istringstream stream(code);
    string line;
    vector<string> cleaned;
    bool skip = false;
    regex rx_module("^module\\s+");

    while (getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        string s = trim(line);
        if (s.empty()) { cleaned.push_back(line); continue; }

        // Strip markdown backticks
        if (!s.empty() && s.front() == '`') { cleaned.push_back(""); continue; }
        if (!s.empty() && s.back() == '`')  { cleaned.push_back(""); continue; }

        if (s.rfind("//", 0) == 0) { cleaned.push_back(line); continue; }

        if (regex_search(s, rx_module)) { skip = true; continue; }
        if (skip && s.rfind("endmodule", 0) == 0) { skip = false; continue; }
        if (skip) continue;

        if (s.rfind("bind ", 0) == 0) continue;
        if (s.find("|->") != string::npos) continue;
        if (s.find("|=>") != string::npos) continue;
        if (s.find("##") != string::npos) continue;
        if (s.find("disable iff") != string::npos) continue;
        if (s.find("throughout") != string::npos) continue;
        if (s.find("assert property") != string::npos) continue;
        if (s.find("cover property") != string::npos) continue;
        if (s.find("property ") != string::npos || s == "endproperty") continue;
        if (s.find("sequence ") != string::npos || s == "endsequence") continue;
        if (s.rfind("endmodule", 0) == 0) continue;

        cleaned.push_back(line);
    }

    string result;
    for (const auto& l : cleaned) result += l + "\n";
    return result;
}

// --- GroqClient ---
class GroqClient {
private:
    string base_url = "https://api.groq.com/openai/v1/chat/completions";
    vector<string> keys;
    string primary_model;
    string extender_model;
    int ki = 0;

public:
    int calls = 0;

    GroqClient(const string& config_path) {
        unsetenv("http_proxy"); unsetenv("https_proxy"); unsetenv("all_proxy");
        unsetenv("HTTP_PROXY"); unsetenv("HTTPS_PROXY"); unsetenv("ALL_PROXY");

        ifstream f(config_path);
        if (!f.is_open()) throw runtime_error("Could not open config file: " + config_path);
        json cfg = json::parse(f);
        f.close();

        // Collect keys from the primary file, then merge keys from a sibling config when
        // running from build/: ./config.json may be a short copy while ../config.json
        // holds the full key list. Deduplicate by key string.
        set<string> seen_keys;
        auto ingest_keys = [&](const json& c) {
            if (!c.contains("groq_api_keys") || !c["groq_api_keys"].is_array()) return;
            for (const auto& key : c["groq_api_keys"]) {
                if (!key.is_string()) continue;
                string k = key.get<string>();
                if (k.empty()) continue;
                if (seen_keys.insert(k).second) keys.push_back(k);
            }
        };
        ingest_keys(cfg);

        string canon_primary;
        try {
            if (fs::exists(config_path))
                canon_primary = fs::weakly_canonical(fs::path(config_path)).string();
        } catch (...) {}

        vector<string> merge_paths;
        if (config_path == "config.json") merge_paths.push_back("../config.json");
        else if (config_path == "../config.json") merge_paths.push_back("config.json");

        for (const string& alt : merge_paths) {
            if (!fs::exists(alt)) continue;
            try {
                if (!canon_primary.empty()) {
                    string ca = fs::weakly_canonical(fs::path(alt)).string();
                    if (ca == canon_primary) continue;
                }
            } catch (...) {}
            ifstream fa(alt);
            if (!fa.is_open()) continue;
            json cfg2 = json::parse(fa);
            size_t before = keys.size();
            ingest_keys(cfg2);
            if (keys.size() > before)
                cout << "    [API] Merged " << (keys.size() - before) << " additional Groq key(s) from "
                     << alt << " (deduped)\n";
        }

        if (keys.empty())
            throw runtime_error("No groq_api_keys in config (after merge): " + config_path);

        cout << "    [API] Using " << keys.size() << " Groq API key(s) for rotation (primary file: "
             << config_path << ")\n";

        primary_model = cfg["primary_model"];
        
        if (cfg.contains("extender_model")) extender_model = cfg["extender_model"];
        else if (cfg.contains("fallback_model")) extender_model = cfg["fallback_model"];
        else extender_model = primary_model;
    }

    string chat(const string& system, const string& user, const string& role = "leader", float temp = 0.2, int max_tok = 4096) {
        calls++;
        string model = (role == "extender") ? extender_model : primary_model;

        if (calls > 1 && calls % 2 == 0) {
            int wait = 20;
            cout << "    [API] Pausing " << wait << "s (call #" << calls << ")...\n";
            this_thread::sleep_for(chrono::seconds(wait));
        }

        json payload = {
            {"model", model},
            {"temperature", temp},
            {"max_tokens", max_tok},
            {"messages", json::array({{{"role", "system"}, {"content", system}}, {{"role", "user"}, {"content", user}}})}
        };

        // Each 429 rotation consumes one loop iteration. With many keys we need enough
        // iterations for several full cycles + sleeps before giving up.
        // Key-rotation policy: try ALL keys once instantly on 429 before sleeping. Only sleep once per full cycle.
        int max_retries = max(48, (int)keys.size() * 12);
        int start_ki = ki;  // remember where this call started so we detect a full rotation
        for (int attempt = 0; attempt < max_retries; ++attempt) {
            cpr::Response r = cpr::Post(cpr::Url{base_url},
                                        cpr::Header{{"Authorization", "Bearer " + keys[ki]}, {"Content-Type", "application/json"}},
                                        cpr::Body{payload.dump()},
                                        cpr::Timeout{90000},
                                        cpr::VerifySsl{false});

            if (r.status_code == 200) {
                json response_json = json::parse(r.text);
                return response_json["choices"][0]["message"]["content"];
            }

            // Helper: extract a readable error message from Groq's JSON body (or raw text).
            auto extract_api_err = [](const string& body) -> string {
                if (body.empty()) return "<empty body>";
                try {
                    json j = json::parse(body);
                    if (j.contains("error")) {
                        const auto& e = j["error"];
                        string msg = e.value("message", "");
                        string code = e.value("code", "");
                        string type = e.value("type", "");
                        string out = msg;
                        if (!code.empty()) out += " [code=" + code + "]";
                        if (!type.empty()) out += " [type=" + type + "]";
                        return out.empty() ? body : out;
                    }
                } catch (...) {}
                return body.substr(0, 500);
            };

            if (r.status_code == 413) {
                cerr << "    [API] 413 body: " << extract_api_err(r.text) << "\n";
                throw runtime_error("Payload Too Large (413). Try reducing batch size.");
            }

            if (r.status_code == 429) {
                int next = (ki + 1) % keys.size();
                if (next != start_ki) {
                    // Still have untried keys in this cycle — rotate instantly, no sleep.
                    cout << "    [API] 429 on key#" << ki << ", rotating to key#" << next << " (no sleep).\n";
                    ki = next;
                    continue;
                }
                // All keys exhausted this cycle — sleep 30s then reset cycle.
                cout << "    [API] All " << keys.size() << " key(s) rate-limited, sleeping 30s (HTTP try "
                     << attempt + 1 << "/" << max_retries << ")...\n";
                this_thread::sleep_for(chrono::seconds(30));
                ki = next;
                start_ki = ki;  // new cycle begins from current key
                continue;
            }

            // 400 = bad request (bad model, malformed body, context overflow, etc.).
            // Retrying the same payload will not fix it. Print the full body and fail fast.
            if (r.status_code == 400) {
                cerr << "    [API] HTTP 400 (Bad Request) on key#" << ki
                     << " model=" << model << "\n"
                     << "    [API] Groq says: " << extract_api_err(r.text) << "\n"
                     << "    [API] Payload size: " << payload.dump().size() << " bytes, "
                     << "system=" << system.size() << " chars, user=" << user.size() << " chars, "
                     << "max_tokens=" << max_tok << "\n";
                throw runtime_error("Groq HTTP 400 (see message above). Check model ID, payload size, or max_tokens.");
            }

            // 401/403 = auth problem on this key. Don't waste time retrying with the same key.
            if (r.status_code == 401 || r.status_code == 403) {
                cerr << "    [API] HTTP " << r.status_code << " (auth) on key#" << ki
                     << ": " << extract_api_err(r.text) << "\n";
                int next = (ki + 1) % keys.size();
                if (next == start_ki) {
                    throw runtime_error("All API keys rejected (HTTP " + to_string(r.status_code) + "). Check config.json.");
                }
                ki = next;
                continue;
            }

            // 5xx or network error — retry with next key.
            cerr << "    [API] Error: HTTP " << r.status_code
                 << " | cpr error: " << r.error.message
                 << " | body: " << extract_api_err(r.text) << "\n";
            ki = (ki + 1) % keys.size();
            this_thread::sleep_for(chrono::seconds(10));
        }
        
        throw runtime_error("API failed after " + to_string(max_retries) + " retries. Groq might be overloaded.");
    }
};

// --- Pipeline Stages ---
json stage1(GroqClient& client, const string& rtl_code, const string& spec_text, const string& work_dir) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "STAGE 1: SPECIFICATION PROCESSING\n";
    cout << string(60, '=') << "\n";

    cout << "\n  [1A] Signal Mapper...\n";
    string sm_system = "You analyze Verilog RTL and extract signals. Respond with ONLY valid JSON, no markdown.";
    string sm_user = "Extract all signals from this Verilog. Return JSON with:\n- module_name (string)\n- ports: array of {name, direction, width, type}\n- internal_signals: array of {name, width, type}\n- clock_signal (string)\n- reset_signal (string)\n- reset_polarity: \"active_high\" or \"active_low\"\n\nVERILOG:\n" + rtl_code + "\n\nRespond ONLY with JSON:";
    string sm_resp = clean_llm(client.chat(sm_system, sm_user, "leader", 0.2));
    json sig_map = json::parse(sm_resp);
    cout << "    " << get_array(sig_map, "ports").size() << " ports, " << get_array(sig_map, "internal_signals").size() << " internals\n";

    cout << "  [1B] Spec Analyzer...\n";
    string sa_system = "You analyze hardware specs. Respond with ONLY valid JSON, no markdown.";
    string sa_user = "Extract signal-wise functional descriptions from this spec. Return JSON with:\n- module_description (string, 1 paragraph)\n- signals: array of {name, functional_description, reset_behavior, dependencies, assertion_hints}\n\nSPECIFICATION:\n" + spec_text + "\n\nRespond ONLY with JSON:";
    string sa_resp = clean_llm(client.chat(sa_system, sa_user, "leader", 0.2));
    json spec_a = json::parse(sa_resp);
    cout << "    " << get_array(spec_a, "signals").size() << " signal descriptions\n";

    cout << "  [1C] Merging into Information Bank...\n";
    string mg_system = "You merge RTL signal data with spec descriptions. Respond with ONLY valid JSON, no markdown.";
    string mg_user = "Merge into unified information bank. Use RTL-accurate signal names.\n\nSIGNAL MAP (from RTL):\n" + sig_map.dump(2) + "\n\nSPEC ANALYSIS (from NL):\n" + spec_a.dump(2) + "\n\nReturn JSON with:\n- module_name, clock, reset, reset_polarity, module_description\n- signals: array of {rtl_name, direction, width, type, functional_description, reset_behavior, dependencies, assertion_hints}\n\nMatch signals by name similarity. Include ALL signals from RTL even if spec doesn't mention them.\nRespond ONLY with JSON:";
    string mg_resp = clean_llm(client.chat(mg_system, mg_user, "leader", 0.2));
    json info_bank = json::parse(mg_resp);

    string ib_path = work_dir + "/information_bank.json";
    ofstream o(ib_path); o << info_bank.dump(2); o.close();

    cout << "    Saved: " << ib_path << " (" << get_array(info_bank, "signals").size() << " signals)\n";
    cout << "    Module: " << get_str(info_bank, "module_name", "top") << "\n";
    string clock = get_str(info_bank, "clock", "clk");
    string reset = get_str(info_bank, "reset", "rst");
    string polarity = get_str(info_bank, "reset_polarity", "active_low");
    cout << "    Clock: " << clock << ", Reset: " << reset << " (" << polarity << ")\n";
    
    return info_bank;
}

string create_signal_batch_prompt(const json& signals, const string& rtl_code, const json& info_bank, bool is_extender) {
    string signal_descriptions = "";
    for (size_t i = 0; i < signals.size(); ++i) {
        auto sig = signals[i];
        string dir = get_str(sig, "direction", "unknown");
        string width = get_str(sig, "width", "?");
        string type = get_str(sig, "type", "?");
        string name = get_str(sig, "rtl_name", "unknown");
        string func = get_str(sig, "functional_description", "unknown");
        
        signal_descriptions += "  - " + name + " [" + width + "-bit " + type + "] (" + dir + ")\n    Function: " + func;
        
        string reset_beh = get_str(sig, "reset_behavior", "");
        if (!reset_beh.empty() && reset_beh != "null") signal_descriptions += "\n    Reset: " + reset_beh;
        
        json deps = get_array(sig, "dependencies");
        if (!deps.empty()) {
            signal_descriptions += "\n    Depends on: ";
            for (size_t d = 0; d < deps.size(); ++d) {
                if (deps[d].is_string()) {
                    signal_descriptions += deps[d].get<string>();
                    if (d < deps.size() - 1) signal_descriptions += ", ";
                }
            }
        }
        if (i < signals.size() - 1) signal_descriptions += "\n";
    }

    string clock = get_str(info_bank, "clock", "clk");
    string reset = get_str(info_bank, "reset", "rst");
    string polarity = get_str(info_bank, "reset_polarity", "active_low");
    string reset_meaning = (polarity == "active_low") ? "rst==0" : "rst==1";

    string instruction;
    if (is_extender) {
        instruction = "Generate 2-3 assertions per signal below. Focus on:\n"
                      "- State transitions (if was in state X with input Y, must go to state Z)\n"
                      "- Registered output tracking (output == $past(source_reg))\n"
                      "- Enable/selection behavior\n"
                      "- Each assertion MUST have a unique label and be inside always @(posedge clk)\n"
                      "- Guard $past with if (f_past_valid)";
    } else {
        instruction = "Generate exactly 3 assertions and 1 cover per signal below:\n"
                      "1. Reset behavior: what value does this signal take when reset was active? Use: if (f_past_valid && !$past(" + reset + ")) assert(signal == reset_value);\n"
                      "2. Functional correctness: does the signal do what the spec says?\n"
                      "3. Relationship with other signals: how does this signal relate to its dependencies?\n"
                      "4. Cover: a reachable condition for this signal\n\n"
                      "You MUST generate for EVERY signal. Do not skip any.";
    }

    return instruction + "\n\nDESIGN: " + get_str(info_bank, "module_name", "top") + 
           "\nCLOCK: " + clock + " (posedge)\nRESET: " + reset + " (" + polarity + " — " + reset_meaning + " means reset active)\n\n" +
           "IMPORTANT: These signals are OUTPUTS or INTERNAL registers. You CAN assert their values.\n" +
           "Input signals (clk, rst, enable, selection, d_in_0, d_in_1) are UNconstrained — NEVER assert their values.\n\n" +
           "SIGNALS TO GENERATE ASSERTIONS FOR:\n" + signal_descriptions + 
           "\n\nRTL CODE:\n" + rtl_code + "\n\nOutput ONLY the assertion code (no module, no bind, no markdown):";
}

string stage2(GroqClient& client, const json& info_bank, const string& rtl_code, const string& work_dir) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "STAGE 2: BATCHED PER-SIGNAL GENERATION\n";
    cout << string(60, '=') << "\n";

    string LEADER_SYSTEM = R"(You generate formal assertions for open-source SymbiYosys.

CRITICAL RULES:
1. NO concurrent SVA: no |-> , |=> , ## , disable iff, property/endproperty, sequence/endsequence
2. ONLY immediate assertions inside always @(posedge clk) blocks
3. All assertions using $past MUST be guarded by: if (f_past_valid)
4. Give each assertion a UNIQUE descriptive label
5. Use ONLY: $past(), $stable(), $rose(), $fell()
6. Use == for comparison, NEVER === (4-state equality not supported)
7. NEVER put if() inside assert(). The if guard goes OUTSIDE: if (cond) label: assert(expr);
8. NEVER assert values of INPUT signals (clk, rst, enable, selection, d_in_0, d_in_1) — inputs are unconstrained
9. Only assert values of OUTPUT and INTERNAL signals (d_o, wr_en, pstate, nstate, selection_buf, wr_en_reg, d_o_reg)
10. NEVER use $isunknown or any SystemVerilog-only functions

CORRECT PATTERNS:
// Reset clears FSM state (pstate is internal reg, assertable)
always @(posedge clk) begin
    if (f_past_valid && !$past(rst))
        a_pstate_reset: assert(pstate == 3'b000);
end

// Output tracks its registered source
always @(posedge clk) begin
    if (f_past_valid)
        a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
end

// Combinational output depends on current input
always @(posedge clk)
    a_wr_en_reg_sel: assert(selection == 1'b0 ? wr_en_reg == 1'b0 : 1'b1);

// State transition with proper guards
always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable)
        && !($past(selection_buf) && !$past(selection))
        && $past(pstate) == 3'b000 && !$past(selection) && $past(d_in_0[0]))
        a_trans_000_to_001: assert(pstate == 3'b001);
end

// Cover statement for reachability
always @(posedge clk) c_pstate_001: cover(pstate == 3'b001);

WRONG — NEVER DO:
assert(clk == 1'b1);              // WRONG: asserting input value
assert(rst == 1'b0);              // WRONG: asserting input value
assert(d_in_0 <= 8'hFF);          // WRONG: trivial, always true
assert(enable == 1'b1);           // WRONG: asserting input value
assert($stable(pstate));          // WRONG: FSM state changes
assert(signal === value);         // WRONG: use == not ===
assert(if (cond) expr);           // WRONG: if goes outside assert
assert($isunknown(signal));       // WRONG: not supported

TAUTOLOGY BAN — NEVER put the asserted expression (or a conjunct equal to it) inside its own guard:
// WRONG — tautology, proves nothing (guard already contains the postcondition):
if (f_past_valid && d_o == 1'b0)        a_bad: assert(d_o == 1'b0);
if (f_past_valid && pstate == 3'b000)   a_bad: assert(pstate == 3'b000);
// CORRECT — guard encodes the PRECONDITION under which the postcondition MUST hold:
if (f_past_valid && !$past(rst))                        a_good: assert(pstate == 3'b000);
if (f_past_valid && $past(rst) && !$past(enable))       a_good: assert(pstate == 3'b000);
// Rule: the guard may describe WHEN (using $past of inputs/state) but MUST NOT
// contain the asserted signal compared against the same value being asserted.

OUTPUT FORMAT: Raw assertion code only. No module. No bind. No endmodule. No markdown. No backticks.)";

    json signals = get_array(info_bank, "signals");
    json assertable_signals = json::array();
    vector<string> input_names, assertable_names;

    for (const auto& s : signals) {
        string dir = get_str(s, "direction", "");
        string name = get_str(s, "rtl_name", "unknown");
        if (dir == "input") input_names.push_back(name);
        else { assertable_signals.push_back(s); assertable_names.push_back(name); }
    }

    cout << "\n  Total signals: " << signals.size() << "\n";
    cout << "  Inputs (skip assertions): " << format_python_list(input_names) << "\n";
    cout << "  Assertable (output/internal): " << format_python_list(assertable_names) << "\n";

    int batch_size = 2;
    int n_batches = ceil((float)assertable_signals.size() / batch_size);
    cout << "  " << assertable_signals.size() << " assertable signals → " << n_batches << " batches of " << batch_size << "\n";

    string clock = get_str(info_bank, "clock", "clk");
    string reset = get_str(info_bank, "reset", "rst");
    string polarity = get_str(info_bank, "reset_polarity", "active_low");
    string reset_val = (polarity == "active_low") ? "!" + reset : reset;

    string all_assertions = "// === Auto-generated formal assertions ===\n";
    all_assertions += "// Inputs (not asserted): " + join_list(input_names) + "\n";
    all_assertions += "// Assertable signals: " + join_list(assertable_names) + "\n\n";
    all_assertions += "reg f_past_valid;\ninitial f_past_valid = 0;\n";
    all_assertions += "always @(posedge " + clock + ") f_past_valid <= 1;\n";
    // BMC reset-completion scaffold: hold reset ACTIVE for the first 2 cycles so the design's
    // (possibly async) reset actually clears FSM/register state from anyinit values before
    // assertions begin checking. Without this, most step-0 asserts spuriously fail on anyinit.
    all_assertions += "reg [1:0] fmsv_init_cnt;\ninitial fmsv_init_cnt = 0;\n";
    all_assertions += "always @(posedge " + clock + ") if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;\n";
    all_assertions += "always @(*) if (fmsv_init_cnt < 2'd2) assume(" + reset_val + ");\n";

    for (int i = 0; i < n_batches; ++i) {
        int start = i * batch_size;
        int end = min((int)assertable_signals.size(), start + batch_size);
        json batch_signals = json::array();
        vector<string> batch_names_vec;
        
        for (int j = start; j < end; ++j) {
            batch_signals.push_back(assertable_signals[j]);
            batch_names_vec.push_back(get_str(assertable_signals[j], "rtl_name", "unknown"));
        }
        
        string batch_names_str = format_python_list(batch_names_vec);
        cout << "\n  [Batch " << i + 1 << "/" << n_batches << "] Signals: " << batch_names_str << "\n";

        cout << "    Leader generating...\n";
        string l_resp = clean_assertions(clean_llm(client.chat(LEADER_SYSTEM, create_signal_batch_prompt(batch_signals, rtl_code, info_bank, false), "leader", 0.2)));
        int l_a = count_regex(l_resp, "\\bassert\\s*\\(");
        int l_c = count_regex(l_resp, "\\bcover\\s*\\(");
        cout << "    Leader: " << l_a << " assertions, " << l_c << " covers\n";
        all_assertions += "\n// --- Batch " + to_string(i + 1) + " Leader ---\n" + l_resp + "\n";

        cout << "    Extender generating...\n";
        string e_resp = clean_assertions(clean_llm(client.chat(LEADER_SYSTEM, create_signal_batch_prompt(batch_signals, rtl_code, info_bank, true), "extender", 0.3)));
        int e_a = count_regex(e_resp, "\\bassert\\s*\\(");
        int e_c = count_regex(e_resp, "\\bcover\\s*\\(");
        cout << "    Extender: " << e_a << " assertions, " << e_c << " covers\n";
        all_assertions += "\n// --- Batch " + to_string(i + 1) + " Extender ---\n" + e_resp + "\n";
    }

    all_assertions = regex_replace(all_assertions, regex("==="), "==");
    istringstream stream(all_assertions);
    string line, final_code;
    vector<string> seen_asserts;
    
    // --- NEW: Regex and Map for Label Uniquification ---
    regex rx_assert("\\bassert\\s*\\("), rx_cover("\\bcover\\s*\\(");
    regex rx_label("([a-zA-Z0-9_]+)\\s*:\\s*(assert|cover)\\b");
    map<string, int> label_counts; 
    // ---------------------------------------------------

    while (getline(stream, line)) {
        if (line.find("$isunknown") != string::npos) continue;
        string stripped = trim(line);
        if (regex_search(stripped, rx_assert) || regex_search(stripped, rx_cover)) {
            string normalized = regex_replace(stripped, regex("\\s+"), " ");
            
            // 1. Check for exact duplicates
            if (find(seen_asserts.begin(), seen_asserts.end(), normalized) != seen_asserts.end()) {
                final_code += "// [DEDUP] " + stripped + "\n        ;\n";
                continue;
            }
            seen_asserts.push_back(normalized);

            // 2. NEW: Ensure unique labels so Yosys doesn't crash
            smatch match;
            if (regex_search(line, match, rx_label)) {
                string label = match[1];
                if (label_counts.count(label)) {
                    label_counts[label]++;
                    string new_label = label + "_" + to_string(label_counts[label]);
                    line = regex_replace(line, regex(label + "\\s*:"), new_label + ":");
                } else {
                    label_counts[label] = 1;
                }
            }
        }
        final_code += line + "\n";
    }

    int total_a = count_regex(final_code, "\\bassert\\s*\\(");
    int total_c = count_regex(final_code, "\\bcover\\s*\\(");
    int dedup_c = count_substring(final_code, "[DEDUP]");

    string raw_path = work_dir + "/assertions_raw_llm.sv";
    ofstream o(raw_path); o << final_code; o.close();

    cout << "\n  " << string(40, '=') << "\n";
    cout << "  STAGE 2 SUMMARY\n";
    cout << "  " << string(40, '=') << "\n";
    cout << "  Total assertions: " << total_a << "\n";
    cout << "  Total covers:     " << total_c << "\n";
    cout << "  Duplicates removed: " << dedup_c << "\n";
    cout << "  Batches processed:  " << n_batches << "\n";
    cout << "  Saved: " << raw_path << "\n";

    return final_code;
}


// --- Stage 3 salvage helpers ---------------------------------------------------
// When auto-fix retries are exhausted, instead of failing the whole pipeline we
// bisect the assertion text into individual blocks and keep only those that pass
// Yosys' syntax check. Bad blocks are dropped; survivors flow to Stage 4.
struct AssertBlock {
    string text;
    bool   has_prop;   // contains assert( or cover( (i.e., it's a candidate to drop)
};

// Word-boundary count of `word` in `s`. Avoids matching "begin" inside "begins"
// or "end" inside "endmodule".
static int s3_count_word(const string& s, const string& word) {
    int n = 0;
    size_t pos = 0;
    while ((pos = s.find(word, pos)) != string::npos) {
        bool left_ok  = (pos == 0) || (!isalnum((unsigned char)s[pos - 1]) && s[pos - 1] != '_');
        size_t after  = pos + word.size();
        bool right_ok = (after >= s.size()) || (!isalnum((unsigned char)s[after]) && s[after] != '_');
        if (left_ok && right_ok) n++;
        pos += word.size();
    }
    return n;
}

// Count "end" tokens that are bare `end` (not endmodule/endcase/endgenerate/etc.).
static int s3_count_bare_end(const string& s) {
    int n = 0;
    size_t pos = 0;
    while ((pos = s.find("end", pos)) != string::npos) {
        bool left_ok  = (pos == 0) || (!isalnum((unsigned char)s[pos - 1]) && s[pos - 1] != '_');
        size_t after  = pos + 3;
        bool right_ok = (after >= s.size()) || (!isalnum((unsigned char)s[after]) && s[after] != '_');
        if (left_ok && right_ok) n++;
        pos += 3;
    }
    return n;
}

// Anchor-based splitter: find every assert(/cover( occurrence and extract its
// full enclosing always/initial statement as one atomic property block. Text
// outside property ranges is emitted as "infra" blocks (reg/wire/initial/
// assume/comments/blanks), which are always kept together during bisection.
//
// This is robust against the common two-line pattern:
//     always @(posedge clk)
//         a_label: assert(...);
// which the previous line-based splitter broke into two orphaned pieces that
// both failed Yosys on their own.
static vector<AssertBlock> s3_split_blocks(const string& assertions) {
    const string& src = assertions;
    const size_t N = src.size();
    vector<AssertBlock> blocks;

    // 1) Find every assert(/cover( token NOT inside a // comment.
    vector<size_t> prop_hits;
    {
        regex rx("\\b(assert|cover)\\s*\\(");
        for (auto it = sregex_iterator(src.begin(), src.end(), rx);
             it != sregex_iterator(); ++it) {
            size_t p = (size_t)it->position();
            size_t line_start = src.rfind('\n', p);
            line_start = (line_start == string::npos) ? 0 : line_start + 1;
            // Skip if the line (up to this token) is commented out.
            string prefix = src.substr(line_start, p - line_start);
            if (prefix.find("//") != string::npos) continue;
            prop_hits.push_back(p);
        }
    }

    // Helper: find start of line containing `pos`.
    auto line_start_of = [&](size_t pos) -> size_t {
        if (pos == 0) return 0;
        size_t nl = src.rfind('\n', pos - 1);
        return (nl == string::npos) ? 0 : nl + 1;
    };

    // Helper: scan back from `pos` to find the most recent line whose first
    // non-whitespace token is `always` or `initial`. Returns that line's start.
    // Falls back to the line containing `pos` if none found within a short window.
    auto find_stmt_start = [&](size_t pos) -> size_t {
        size_t cursor = line_start_of(pos);
        size_t fallback = cursor;
        // Walk back up to 10 lines looking for always/initial header.
        for (int steps = 0; steps < 12 && cursor > 0; ++steps) {
            string line = src.substr(cursor, (src.find('\n', cursor) == string::npos
                                               ? N : src.find('\n', cursor)) - cursor);
            string t = trim(line);
            // Hit the procedural header — use it.
            if (t.rfind("always", 0) == 0 || t.rfind("initial", 0) == 0)
                return cursor;
            // Blank/comment boundary: stop (don't pull in unrelated lines).
            if (t.empty() || t.rfind("//", 0) == 0) return fallback;
            fallback = cursor;
            // Step back one line.
            if (cursor == 0) break;
            size_t prev = src.rfind('\n', cursor - 1);
            cursor = (prev == string::npos) ? 0 : prev + 1;
        }
        return fallback;
    };

    // Helper: scan forward from `start` to find the end of the statement.
    // Tracks paren depth and begin/end depth, skipping // comments.
    // Returns exclusive end offset.
    auto find_stmt_end = [&](size_t start) -> size_t {
        int paren = 0, brace = 0;
        bool entered_block = false;
        size_t i = start;
        while (i < N) {
            char c = src[i];
            // Line comments.
            if (c == '/' && i + 1 < N && src[i + 1] == '/') {
                size_t nl = src.find('\n', i);
                i = (nl == string::npos) ? N : nl; // include newline on next iter
                continue;
            }
            // Block comments (rare but cheap to handle).
            if (c == '/' && i + 1 < N && src[i + 1] == '*') {
                size_t end = src.find("*/", i + 2);
                i = (end == string::npos) ? N : end + 2;
                continue;
            }
            if (c == '(') { paren++; i++; continue; }
            if (c == ')') { paren--; i++; continue; }
            // Word-level begin/end detection.
            if (isalpha((unsigned char)c) || c == '_') {
                size_t j = i;
                while (j < N && (isalnum((unsigned char)src[j]) || src[j] == '_')) j++;
                string w = src.substr(i, j - i);
                if (w == "begin") { brace++; entered_block = true; }
                else if (w == "end") {
                    brace--;
                    if (entered_block && brace <= 0) {
                        // Include trailing ';' if present and the rest of the line.
                        size_t k = j;
                        while (k < N && (src[k] == ' ' || src[k] == '\t')) k++;
                        if (k < N && src[k] == ';') k++;
                        // Consume to end-of-line so we don't leave a dangling fragment.
                        size_t nl = src.find('\n', k);
                        return (nl == string::npos) ? N : nl + 1;
                    }
                }
                i = j;
                continue;
            }
            if (c == ';' && paren == 0 && brace == 0 && !entered_block) {
                size_t nl = src.find('\n', i);
                return (nl == string::npos) ? N : nl + 1;
            }
            i++;
        }
        return N;
    };

    // 2) For each prop hit, compute [start, end) range.
    vector<pair<size_t, size_t>> ranges;
    ranges.reserve(prop_hits.size());
    for (size_t p : prop_hits) {
        size_t s = find_stmt_start(p);
        size_t e = find_stmt_end(s);
        if (e <= p) e = min(N, p + 1);
        ranges.push_back({s, e});
    }

    // 3) Merge overlapping/adjacent ranges (multiple asserts in one begin..end).
    sort(ranges.begin(), ranges.end());
    vector<pair<size_t, size_t>> merged;
    for (auto& r : ranges) {
        if (!merged.empty() && r.first <= merged.back().second) {
            merged.back().second = max(merged.back().second, r.second);
        } else {
            merged.push_back(r);
        }
    }

    // 4) Emit infra/property blocks in source order.
    size_t cursor = 0;
    for (auto& [s, e] : merged) {
        if (s > cursor) {
            AssertBlock ib;
            ib.text = src.substr(cursor, s - cursor);
            ib.has_prop = false;
            if (!ib.text.empty()) blocks.push_back(ib);
        }
        AssertBlock pb;
        pb.text = src.substr(s, e - s);
        if (!pb.text.empty() && pb.text.back() != '\n') pb.text += '\n';
        pb.has_prop = true;
        blocks.push_back(pb);
        cursor = e;
    }
    if (cursor < N) {
        AssertBlock ib;
        ib.text = src.substr(cursor);
        ib.has_prop = false;
        blocks.push_back(ib);
    }

    return blocks;
}

static string s3_assemble(const vector<AssertBlock>& bs) {
    string out;
    for (const auto& b : bs) out += b.text;
    return out;
}

// Inject `assertions` into `rtl_code`, write to `path`, run Yosys, return true on clean compile.
static bool s3_try_compile(const string& rtl_code, const string& assertions,
                           const string& module_name, const string& path) {
    string injected = rtl_code;
    size_t pos = injected.rfind("endmodule");
    if (pos != string::npos)
        injected.insert(pos, "\n// --- INJECTED FMSV ASSERTIONS ---\n" + assertions + "\n");
    else
        injected += "\n// --- INJECTED FMSV ASSERTIONS ---\n" + assertions;
    { ofstream o(path); o << injected; }
    string cmd = "yosys -p \"read_verilog -sv " + path + "; prep -top " + module_name + "\"";
    string out = exec_cmd(cmd);
    return out.find("ERROR:") == string::npos && out.find("Syntax error") == string::npos;
}

struct BisectStats { int kept = 0; int dropped = 0; int probes = 0; };

// Bisect over `props`, keeping `infra` always included. Returns surviving prop blocks.
// Strategy: if the whole subset compiles, keep it. Else if singleton, drop it.
// Else split in half and recurse on each half independently. After recursion,
// re-verify the union compiles together (catches rare cross-block interactions
// like duplicate labels) and falls back to one half if not.
static vector<AssertBlock> s3_bisect(const string& rtl_code,
                                     const vector<AssertBlock>& infra,
                                     const vector<AssertBlock>& props,
                                     const string& module_name,
                                     const string& work_dir,
                                     BisectStats& stats) {
    if (props.empty()) return props;
    string probe_path = work_dir + "/dut_with_asserts_bisect.sv";

    string combined_text = s3_assemble(infra) + s3_assemble(props);
    stats.probes++;
    if (s3_try_compile(rtl_code, combined_text, module_name, probe_path)) {
        stats.kept += (int)props.size();
        return props;
    }

    if (props.size() == 1) {
        stats.dropped++;
        return {};
    }

    size_t mid = props.size() / 2;
    vector<AssertBlock> left (props.begin(), props.begin() + mid);
    vector<AssertBlock> right(props.begin() + mid, props.end());

    vector<AssertBlock> surv_left  = s3_bisect(rtl_code, infra, left,  module_name, work_dir, stats);
    vector<AssertBlock> surv_right = s3_bisect(rtl_code, infra, right, module_name, work_dir, stats);

    vector<AssertBlock> combined;
    combined.insert(combined.end(), surv_left.begin(),  surv_left.end());
    combined.insert(combined.end(), surv_right.begin(), surv_right.end());

    if (combined.empty()) return combined;

    string check_text = s3_assemble(infra) + s3_assemble(combined);
    stats.probes++;
    if (s3_try_compile(rtl_code, check_text, module_name, probe_path)) {
        return combined;
    }

    // Cross-block interaction (e.g. duplicate label across halves). Keep the larger
    // independently-compiling half so we still flow something to Stage 4.
    return surv_left.size() >= surv_right.size() ? surv_left : surv_right;
}

bool stage3(GroqClient& client, const string& rtl_code, string& assertions, const json& info_bank, const string& work_dir) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "STAGE 3: YOSYS COMPILATION & AUTO-FIX\n";
    cout << string(60, '=') << "\n";

    string module_name = get_str(info_bank, "module_name", "top");
    int max_retries = 3;

    for (int attempt = 1; attempt <= max_retries; ++attempt) {
        cout << "\n  [Compile Attempt " << attempt << "/" << max_retries << "] Injecting assertions into DUT...\n";
        
        string injected_rtl = rtl_code;
        size_t pos = injected_rtl.rfind("endmodule");
        if (pos != string::npos) {
            injected_rtl.insert(pos, "\n// --- INJECTED FMSV ASSERTIONS ---\n" + assertions + "\n");
        } else {
            injected_rtl += "\n// --- INJECTED FMSV ASSERTIONS ---\n" + assertions;
        }

        string dut_path = work_dir + "/dut_with_asserts.sv";
        ofstream o(dut_path); o << injected_rtl; o.close();

        cout << "    Running Yosys compiler syntax check...\n";
        string cmd = "yosys -p \"read_verilog -sv " + dut_path + "; prep -top " + module_name + "\"";
        string yosys_out = exec_cmd(cmd);

        if (yosys_out.find("ERROR:") == string::npos && yosys_out.find("Syntax error") == string::npos) {
            cout << "    [SUCCESS] Yosys compiled the injected RTL perfectly!\n";
            
            ofstream final_o(work_dir + "/assertions_compiled_clean.sv"); 
            final_o << assertions; 
            final_o.close();

            cout << "    Saved: " << work_dir << "/assertions_compiled_clean.sv\n";
            return true;
        }

        cout << "    [FAILED] Yosys detected a syntax error in the assertions.\n";

        if (attempt == max_retries) {
            // Auto-fix exhausted. Instead of failing the whole pipeline, bisect
            // the assertion set: keep blocks that compile, drop those that don't,
            // and pass the survivors to Stage 4.
            cout << "    [Auto-Fix exhausted] Bisecting to salvage compiling assertions...\n";

            vector<AssertBlock> all_blocks = s3_split_blocks(assertions);
            vector<AssertBlock> infra, props;
            for (auto& b : all_blocks) {
                if (b.has_prop) props.push_back(b);
                else            infra.push_back(b);
            }
            cout << "    Parsed " << infra.size() << " infrastructure block(s) and "
                 << props.size() << " property block(s).\n";

            if (props.empty()) {
                cout << "    [FATAL] No property blocks found to salvage.\n";
                return false;
            }

            // Sanity probe: if infra alone doesn't compile, the bisect will drop
            // everything vacuously. Emit a clear message instead of a silent massacre.
            {
                string infra_probe_path = work_dir + "/dut_with_asserts_bisect.sv";
                string infra_text = s3_assemble(infra);
                if (!s3_try_compile(rtl_code, infra_text, module_name, infra_probe_path)) {
                    cout << "    [FATAL] Infrastructure block alone fails Yosys syntax check.\n"
                         << "            Auto-fix likely corrupted reg/initial setup; cannot salvage.\n";
                    return false;
                }
            }

            BisectStats stats;
            vector<AssertBlock> survivors = s3_bisect(rtl_code, infra, props, module_name, work_dir, stats);
            int dropped = (int)props.size() - (int)survivors.size();

            cout << "    [Bisect Result] Kept " << survivors.size() << "/" << props.size()
                 << " property blocks (dropped " << dropped << ", "
                 << stats.probes << " Yosys probes).\n";

            if (survivors.empty()) {
                cout << "    [FATAL] All property blocks failed syntax — nothing to pass to Stage 4.\n";
                return false;
            }

            // Reassemble with a marker comment so the salvage is visible in the artifact.
            string salvaged = s3_assemble(infra)
                            + "\n// === [STAGE 3 SALVAGE] " + to_string(dropped)
                            + " of " + to_string(props.size())
                            + " blocks dropped due to unfixable syntax errors ===\n"
                            + s3_assemble(survivors);
            assertions = salvaged;

            ofstream final_o(work_dir + "/assertions_compiled_clean.sv");
            final_o << salvaged;
            final_o.close();

            cout << "    [SUCCESS] Saved salvaged assertions: " << work_dir
                 << "/assertions_compiled_clean.sv (continuing to Stage 4)\n";
            return true;
        }

        cout << "    [Auto-Fix] Sending compiler trace back to LLM...\n";

        string error_tail = yosys_out.length() > 2000
            ? yosys_out.substr(yosys_out.length() - 2000)
            : yosys_out;

        string fix_system = "You are an expert Silicon Verification Engineer. Fix SystemVerilog compilation errors.";

        string fix_user =
            "RTL with assertions:\n```verilog\n" + injected_rtl + "\n```\n\n"
            "Yosys error:\n```\n" + error_tail + "\n```\n\n"
            "Fix ONLY the assertions. Output raw assertion code.";

        string fixed_resp = clean_llm(client.chat(fix_system, fix_user, "leader", 0.1));
        string fixed_clean = clean_assertions(fixed_resp);

        // Defensive guard: if the auto-fix response has drastically fewer assertions than the
        // input, the LLM is stripping rather than fixing. Reject the stripping fix and keep
        // the previous content — we'd rather fail syntax check than pass with zero assertions.
        int prev_a = count_regex(assertions, "\\bassert\\s*\\(") + count_regex(assertions, "\\bcover\\s*\\(");
        int new_a  = count_regex(fixed_clean, "\\bassert\\s*\\(") + count_regex(fixed_clean, "\\bcover\\s*\\(");
        if (prev_a >= 3 && new_a < prev_a / 2) {
            cout << "    [Auto-Fix REJECTED] LLM stripped " << prev_a << "→" << new_a
                 << " properties (suspected over-strip). Keeping previous set and retrying.\n";
            // Keep assertions unchanged — next attempt will re-try Yosys on same content,
            // which will fail again, but that's better than passing with zero properties.
        } else {
            assertions = fixed_clean;
        }
    }

    return false;
}

bool stage4(const string& work_dir, const string& module_name, const string& rtl_code,
            const string& origin_tag = "stage_4_initial") {
    cout << "\n" << string(60, '=') << "\n";
    cout << "STAGE 4: EXHAUSTIVE FORMAL VERIFICATION\n";
    cout << "Isolating and testing every assertion independently...\n";
    cout << string(60, '=') << "\n";
    cout << "  [Provenance] origin_tag = " << origin_tag << "\n";

    // Fresh full run starts fresh history — prevents stale origins from a prior aborted run.
    // Resume flows (--stage 5, --stage 6) bypass this because they never re-enter with initial tag.
    if (origin_tag == "stage_4_initial") {
        string hist_path = work_dir + "/stage4_history.json";
        if (fs::exists(hist_path)) fs::remove(hist_path);
    }

    // 1. Read the clean assertions generated in Stage 3
    ifstream assert_file(work_dir + "/assertions_compiled_clean.sv");
    if (!assert_file.is_open()) throw runtime_error("Could not find assertions_compiled_clean.sv");
    
    vector<string> assertion_lines;
    string line;
    vector<int> target_indices; 
    int current_index = 0;

    while (getline(assert_file, line)) {
        assertion_lines.push_back(line);
        // Identify lines that contain an actual assertion OR a cover statement
        if ((line.find("assert") != string::npos || line.find("cover") != string::npos) && line.find("//") == string::npos) {
            target_indices.push_back(current_index);
        }
        current_index++;
    }

    int total_asserts = target_indices.size();
    cout << "  Found " << total_asserts << " unique assertions to test.\n\n";

    // Empty set is not "all pass" — it's an error (Stage 3 over-stripped). Write empty results
    // with a sentinel so Stage 5 can detect this and restore the previous round.
    if (total_asserts == 0) {
        cout << "  [ABORT] Zero assertions found — treating as Stage 3 failure.\n";
        json results = { {"passed", json::array()}, {"failed", json::array()}, {"empty", true} };
        ofstream res_o(work_dir + "/stage4_results.json"); res_o << results.dump(2);
        return false;
    }

    json passed_list = json::array();
    json failed_list = json::array();

    // Create a directory to hold the counter-example traces for Stage 5
    string traces_dir = work_dir + "/failed_traces";
    fs::create_directories(traces_dir);

    // Regex to extract the failure step from the SBY log
    regex rx_step("step (\\d+)");
    // Regex to extract highest checked step on PASS (BMC log has "Checking assertions in step N..")
    regex rx_check_step("Checking assert[^ ]* in step (\\d+)");
    const int kBmcDepth = 20;  // must match [options] depth below

    // 2. The Isolation Loop
    for (int i = 0; i < total_asserts; ++i) {
        int active_idx = target_indices[i];
        
        string active_line = assertion_lines[active_idx];
        smatch match;
        string label_name = "unknown_assert_" + to_string(i);
        if (regex_search(active_line, match, regex("([a-zA-Z0-9_]+)\\s*:"))) {
            label_name = match[1];
        }

        cout << "  [" << (i + 1) << "/" << total_asserts << "] Testing: " << label_name << "... ";

        // Build a temporary assertions string where ONLY the active_idx is uncommented
        string isolated_asserts = "";
        for (int j = 0; j < assertion_lines.size(); ++j) {
            if (find(target_indices.begin(), target_indices.end(), j) != target_indices.end() && j != active_idx) {
                // Use a null statement that satisfies 'if' blocks but does nothing
                isolated_asserts += "        begin end // [ISOLATED] " + trim(assertion_lines[j]) + "\n";
            } else {
                isolated_asserts += assertion_lines[j] + "\n";
            }
        }

        string injected_rtl = rtl_code;
        size_t pos = injected_rtl.rfind("endmodule");
        if (pos != string::npos) injected_rtl.insert(pos, "\n// --- INJECTED FMSV ASSERTIONS ---\n" + isolated_asserts + "\n");
        else injected_rtl += "\n// --- INJECTED FMSV ASSERTIONS ---\n" + isolated_asserts;

        ofstream o(work_dir + "/dut_temp.sv"); o << injected_rtl; o.close();

        string sby_content = 
            "[options]\nmode bmc\ndepth 20\n\n[engines]\nsmtbmc z3\n\n"
            "[script]\nread_verilog -sv dut_temp.sv\nprep -top " + module_name + "\n\n"
            "[files]\ndut_temp.sv\n";
        
        ofstream config_o(work_dir + "/temp.sby"); config_o << sby_content; config_o.close();

        string sby_cmd = "export PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" && cd " + work_dir + " && sby -f temp.sby > temp_sby.log 2>&1";
        system(sby_cmd.c_str());

        ifstream log_in(work_dir + "/temp_sby.log");
        string log_content((istreambuf_iterator<char>(log_in)), istreambuf_iterator<char>());

        // Remove the closing parentheses in the search strings
        if (log_content.find("DONE (PASS") != string::npos) {
            // Walk every "Checking ... in step N" line and keep the max — this is the
            // highest cycle BMC actually reached while the property still held.
            int max_reached = -1;
            auto it = sregex_iterator(log_content.begin(), log_content.end(), rx_check_step);
            auto end = sregex_iterator();
            for (; it != end; ++it) {
                try { int v = stoi((*it)[1].str()); if (v > max_reached) max_reached = v; }
                catch (...) {}
            }
            int proven_depth = (max_reached >= 0) ? (max_reached + 1) : kBmcDepth;
            cout << "PASS (depth=" << proven_depth << ")\n";
            passed_list.push_back({
                {"label", label_name},
                {"original_code", trim(active_line)},
                {"bmc_depth", kBmcDepth},
                {"proven_depth", proven_depth}
            });
        } else if (log_content.find("DONE (FAIL") != string::npos) {
            // Extract the cycle step where it failed
            string fail_step = "unknown";
            smatch step_match;
            if (regex_search(log_content, step_match, rx_step)) {
                fail_step = step_match[1];
            }
            
            cout << "FAIL (at Step " << fail_step << ")\n";
            
            // Save the specific trace (both VCD for waveform viewers AND trace_tb.v for LLM consumption)
            string trace_vcd    = label_name + "_trace.vcd";
            string trace_tb_sv  = label_name + "_trace_tb.v";
            string copy_vcd = "cp " + work_dir + "/temp/engine_0/trace.vcd "   + traces_dir + "/" + trace_vcd    + " 2>/dev/null";
            string copy_tb  = "cp " + work_dir + "/temp/engine_0/trace_tb.v "  + traces_dir + "/" + trace_tb_sv  + " 2>/dev/null";
            system(copy_vcd.c_str());
            system(copy_tb.c_str());

            // Build rich JSON object for Stage 5
            failed_list.push_back({
                {"label", label_name},
                {"failed_step", fail_step},
                {"trace_file", "failed_traces/" + trace_vcd},
                {"trace_tb_file", "failed_traces/" + trace_tb_sv},
                {"original_code", trim(active_line)}
            });

        } else {
            cout << "ERROR (Syntax/Vacuity)\n";
            failed_list.push_back({
                {"label", label_name},
                {"error", "Engine crashed or found vacuity"},
                {"original_code", trim(active_line)}
            });
        }
    }

    // 3. Origin tracking — merge with stage4_history.json so each label keeps the EARLIEST
    //    stage it first passed. Later rounds that re-prove the same label inherit the original
    //    origin; genuinely-new passes get tagged with the current origin_tag.
    {
        string hist_path = work_dir + "/stage4_history.json";
        json history = json::object();
        if (fs::exists(hist_path)) {
            try { history = load_json_file(hist_path); if (!history.is_object()) history = json::object(); }
            catch (...) { history = json::object(); }
        }
        for (auto& p : passed_list) {
            string lbl = p.value("label", "");
            if (lbl.empty()) continue;
            int cur_depth = p.value("proven_depth", kBmcDepth);
            if (!history.contains(lbl)) {
                history[lbl] = {
                    {"origin", origin_tag},
                    {"first_proven_depth", cur_depth},
                    {"best_proven_depth", cur_depth}
                };
            } else {
                // Keep earliest origin; update best-ever depth if this run proved deeper.
                int best = history[lbl].value("best_proven_depth", 0);
                if (cur_depth > best) history[lbl]["best_proven_depth"] = cur_depth;
            }
            // Enrich the passed entry with stable provenance so downstream stages can read it.
            p["origin"] = history[lbl].value("origin", origin_tag);
            p["first_proven_depth"] = history[lbl].value("first_proven_depth", cur_depth);
            p["best_proven_depth"]  = history[lbl].value("best_proven_depth",  cur_depth);
        }
        ofstream hf(hist_path); hf << history.dump(2);
    }

    // 4. Save the separated buckets for Stage 5
    json results = {
        {"passed", passed_list},
        {"failed", failed_list}
    };
    ofstream res_o(work_dir + "/stage4_results.json"); res_o << results.dump(2); res_o.close();

    cout << "\n  " << string(40, '=') << "\n";
    cout << "  STAGE 4 SUMMARY\n";
    cout << "  " << string(40, '=') << "\n";
    cout << "  Total Tested: " << total_asserts << "\n";
    cout << "  Passed:       " << passed_list.size() << "\n";
    cout << "  Failed:       " << failed_list.size() << "\n";
    cout << "  Results JSON: " << work_dir << "/stage4_results.json\n";

    return failed_list.empty();
}

json stage5(GroqClient& client, const string& work_dir, const string& rtl_code,
            const string& module_name, const json& info_bank, int max_rounds) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "STAGE 5: ASSERTION REFINEMENT (LLM + YOSYS + BMC LOOP)\n";
    cout << string(60, '=') << "\n";

    const string LEADER_SYSTEM = R"(You generate formal assertions for open-source SymbiYosys.

CRITICAL RULES:
1. NO concurrent SVA: no |-> , |=> , ## , disable iff, property/endproperty, sequence/endsequence
2. ONLY immediate assertions inside always @(posedge clk) blocks
3. All assertions using $past MUST be guarded by: if (f_past_valid)
4. Give each assertion a UNIQUE descriptive label
5. Use ONLY: $past(), $stable(), $rose(), $fell()
6. Use == for comparison, NEVER === (4-state equality not supported)
7. NEVER put if() inside assert(). The if guard goes OUTSIDE: if (cond) label: assert(expr);
8. NEVER assert values of INPUT signals (clk, rst, enable, selection, d_in_0, d_in_1) — inputs are unconstrained
9. Only assert values of OUTPUT and INTERNAL signals (d_o, wr_en, pstate, nstate, selection_buf, wr_en_reg, d_o_reg)
10. NEVER use $isunknown or any SystemVerilog-only functions

CORRECT PATTERNS:
// Reset clears FSM state (pstate is internal reg, assertable)
always @(posedge clk) begin
    if (f_past_valid && !$past(rst))
        a_pstate_reset: assert(pstate == 3'b000);
end

// Output tracks its registered source
always @(posedge clk) begin
    if (f_past_valid)
        a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
end

// Combinational output depends on current input
always @(posedge clk)
    a_wr_en_reg_sel: assert(selection == 1'b0 ? wr_en_reg == 1'b0 : 1'b1);

// State transition with proper guards
always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable)
        && !($past(selection_buf) && !$past(selection))
        && $past(pstate) == 3'b000 && !$past(selection) && $past(d_in_0[0]))
        a_trans_000_to_001: assert(pstate == 3'b001);
end

// Cover statement for reachability
always @(posedge clk) c_pstate_001: cover(pstate == 3'b001);

WRONG — NEVER DO:
assert(clk == 1'b1);              // WRONG: asserting input value
assert(rst == 1'b0);              // WRONG: asserting input value
assert(d_in_0 <= 8'hFF);          // WRONG: trivial, always true
assert(enable == 1'b1);           // WRONG: asserting input value
assert($stable(pstate));          // WRONG: FSM state changes
assert(signal === value);         // WRONG: use == not ===
assert(if (cond) expr);           // WRONG: if goes outside assert
assert($isunknown(signal));       // WRONG: not supported

TAUTOLOGY BAN — NEVER put the asserted expression (or a conjunct equal to it) inside its own guard:
// WRONG — tautology, proves nothing (guard already contains the postcondition):
if (f_past_valid && d_o == 1'b0)        a_bad: assert(d_o == 1'b0);
if (f_past_valid && pstate == 3'b000)   a_bad: assert(pstate == 3'b000);
// CORRECT — guard encodes the PRECONDITION under which the postcondition MUST hold:
if (f_past_valid && !$past(rst))                        a_good: assert(pstate == 3'b000);
if (f_past_valid && $past(rst) && !$past(enable))       a_good: assert(pstate == 3'b000);
// Rule: the guard may describe WHEN (using $past of inputs/state) but MUST NOT
// contain the asserted signal compared against the same value being asserted.

OUTPUT FORMAT: Raw assertion code only. No module. No bind. No endmodule. No markdown. No backticks.)";

    // --- THE VAULT: rebuilt fresh from stage4_results.json passed[] at the start of each round. ---
    json s4_init = load_json_file(work_dir + "/stage4_results.json");
    if (get_array(s4_init, "failed").empty()) {
        cout << "  All assertions PASSED (nothing to refine)!\n";
        return json{{"status", "PASS"}, {"rounds", 0}};
    }

    string locked_passes;
    set<string> vault_labels;

    const size_t kMaxVaultInPrompt    = 8000;
    const size_t kMaxFailedCode       = 4000;
    const size_t kMaxTracesTotal      = 10000;
    const size_t kMaxRtlInPrompt      = 14000;
    const int    kMaxFailsWithTrace   = 8;
    const int    kMaxCycles           = 8;

    // Cumulative best snapshot of ALL passing asserts ever seen (used only to RESTORE after a
    // round that regressed; the in-prompt vault is still round-local to avoid contradictions).
    string best_locked_passes;
    set<string> best_vault_labels;
    int best_assert_passes = -1;

    string design_hints = build_design_hints(info_bank);

    for (int round = 1; round <= max_rounds; ++round) {
        cout << "\n  --- Refinement Round " << round << "/" << max_rounds << " ---\n";

        json stage4_results = load_json_file(work_dir + "/stage4_results.json");
        json failed = get_array(stage4_results, "failed");
        json passed = get_array(stage4_results, "passed");

        if (failed.empty()) {
            cout << "  All assertions PASSED!\n";
            return json{{"status", "PASS"}, {"rounds", round - 1}, {"vault_labels", vault_labels.size()}};
        }

        // Fresh rebuild: asserts only, no covers. Drops labels that regressed.
        // Two vaults: (1) asserts-only for the PROMPT (keeps tokens tight and avoids
        // polluting the LLM context with trivial reachability probes); (2) full vault
        // including covers for INJECTION into the DUT, so Stage 6 can still report
        // cover coverage for the reachability-only signals.
        int seeded = vault_rebuild_from_passed(locked_passes, vault_labels, passed, /*include_covers=*/false);
        string locked_full; set<string> vault_full_labels;
        int seeded_full = vault_rebuild_from_passed(locked_full, vault_full_labels, passed, /*include_covers=*/true);
        cout << "  [Vault] Rebuilt: " << seeded << " asserts for prompt, " << seeded_full << " props (incl. covers) for injection\n";
        {
            ofstream vf(work_dir + "/stage5_vault_passes.sv");
            vf << "// Stage 5 vault — current-round BMC-passed ASSERTS (covers excluded)\n" << locked_passes;
        }

        int fail_before = (int)failed.size();

        // Build per-failure blocks with compact readable traces from trace_tb.v.
        string failed_code;
        string failed_labels;
        string traces;
        int trace_count = 0;
        for (auto& f : failed) {
            if (!f.is_object()) continue;
            string label = get_str(f, "label", "unknown");
            failed_labels += label + "\n";
            string step = "unknown";
            if (f.contains("failed_step") && f["failed_step"].is_string())
                step = f["failed_step"].get<string>();
            else if (f.contains("error"))
                step = get_str(f, "error", "unknown");
            failed_code += "// FAILED at step " + step + "\n";
            if (f.contains("original_code") && f["original_code"].is_string())
                failed_code += f["original_code"].get<string>() + "\n";

            if (trace_count >= kMaxFailsWithTrace) continue;
            string tb = get_str(f, "trace_tb_file", "");
            if (!tb.empty()) {
                string path = work_dir + "/" + tb;
                if (fs::exists(path)) {
                    string summary = summarize_trace_tb(path, kMaxCycles);
                    if (!summary.empty()) {
                        traces += "// Counterexample inputs for " + label + " (failed at step " + step + "):\n"
                               +  summary + "\n";
                        trace_count++;
                    }
                }
            }
        }
        if (traces.empty()) traces = "  (no readable traces available — rely on RTL + failed code)\n";

        string real_uncovered = build_real_uncovered_hint(info_bank, locked_passes);

        string vault_block  = truncate_prompt_section(locked_passes, kMaxVaultInPrompt);
        string failed_block = truncate_prompt_section(failed_code,  kMaxFailedCode);
        string traces_block = truncate_prompt_section(traces,       kMaxTracesTotal);
        string rtl_block    = "```verilog\n" + truncate_prompt_section(rtl_code, kMaxRtlInPrompt) + "\n```\n";

        // Temperature ramps so later rounds don't just re-emit the same broken assertions.
        float temps[] = {0.2f, 0.4f, 0.55f, 0.7f};
        float temp = temps[min(round - 1, 3)];

        string user_prompt =
            "You are refining formal assertions for an RTL design. Some assertions FAIL BMC; help fix them.\n\n"
            "=== DESIGN CONTEXT ===\n" + design_hints + "\n"
            "=== ASSERTIONS ALREADY PROVEN (reference only — do NOT repeat in output) ===\n" + vault_block + "\n"
            "=== FAILED ASSERTIONS (these need to be fixed or dropped) ===\n" + failed_block + "\n"
            "=== COUNTEREXAMPLE INPUT SEQUENCES (why each failed) ===\n" + traces_block + "\n"
            "=== SIGNALS STILL LACKING A PASSING ASSERT ===\n" + real_uncovered + "\n"
            "=== RTL (authoritative) ===\n" + rtl_block + "\n"
            "INSTRUCTIONS:\n"
            "1. Output ONLY new/replacement assertion code for the FAILED labels (and optional 2-3 NEW asserts\n"
            "   targeting the signals listed as 'STILL LACKING A PASSING ASSERT').\n"
            "2. Do NOT reprint the proven vault — it is concatenated automatically.\n"
            "3. Use the counterexample input sequence for each failure to add the exact guard condition\n"
            "   that would make the assertion hold (e.g. if ($past(rst) && $past(enable) && ...)).\n"
            "4. If an assertion is fundamentally wrong (contradicts the RTL), DROP it — don't force a fix.\n"
            "5. Each assertion MUST be wrapped in always @(posedge clk) begin ... end with an `if` guard\n"
            "   outside the assert(). Use f_past_valid before any $past().\n"
            "6. Use == not ===. No concurrent SVA (|-> |=> ##). No $isunknown. No property/endproperty.\n"
            "7. Give each output assertion a UNIQUE label (e.g., a_<signal>_<context>_r" + to_string(round) + ").\n"
            "8. Output raw Verilog. No markdown, no backticks, no module header.\n";

        { ofstream p(work_dir + "/stage5_last_user_prompt.txt"); p << user_prompt; }
        cout << "  [Stage 5] Prompt ~" << user_prompt.size() << " bytes, temp=" << temp
             << ", failed=" << fail_before << ", vault=" << vault_labels.size() << "\n";

        string llm_out = clean_assertions(clean_llm(client.chat(LEADER_SYSTEM, user_prompt, "leader", temp)));
        { ofstream o(work_dir + "/stage5_llm_raw_r" + to_string(round) + ".sv"); o << llm_out; }

        // Rebuild the formal preamble every round. Without this, f_past_valid is implicitly
        // declared as an undriven wire and BMC can set it to 1 at step 0 — which makes every
        // guarded assertion fire on step 0 with anyinit register values and fail.
        string clk_name = get_str(info_bank, "clock", "clk");
        string rst_name = get_str(info_bank, "reset", "rst");
        string pol      = get_str(info_bank, "reset_polarity", "active_low");
        string rst_active_expr = (pol == "active_low") ? ("!" + rst_name) : rst_name;

        string preamble;
        preamble += "// ---- Stage 5 formal preamble (regenerated every round) ----\n";
        preamble += "reg f_past_valid;\n";
        preamble += "initial f_past_valid = 0;\n";
        preamble += "always @(posedge " + clk_name + ") f_past_valid <= 1;\n";
        // BMC reset-completion scaffold: hold reset ACTIVE for 2 cycles so the design's
        // async reset actually clears FSM/register state before assertions start checking.
        preamble += "reg [1:0] fmsv_init_cnt;\n";
        preamble += "initial fmsv_init_cnt = 0;\n";
        preamble += "always @(posedge " + clk_name + ") if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;\n";
        preamble += "always @(*) if (fmsv_init_cnt < 2'd2) assume(" + rst_active_expr + ");\n";
        preamble += "\n";

        // The full assertion set for this round = preamble + vault (proven, incl. covers) + LLM refinements.
        string locked_wrapped = wrap_bare_asserts(locked_full, clk_name);
        string combined = preamble
                        + locked_wrapped
                        + "\n// ---- Stage 5 round " + to_string(round) + " refinements ----\n"
                        + llm_out;

        // Save previous stage4 results so we can restore if this round produces garbage.
        json s4_snapshot = load_json_file(work_dir + "/stage4_results.json");

        bool stg3_pass = stage3(client, rtl_code, combined, info_bank, work_dir);
        if (!stg3_pass) {
            cout << "  Stage 3 failed in round " << round << " — skipping BMC, trying next round.\n";
            continue;
        }

        // Sanity check: stage3 may over-strip and produce 0 assertions. Detect and restore.
        {
            ifstream ck(work_dir + "/assertions_compiled_clean.sv");
            string clean_content((istreambuf_iterator<char>(ck)), istreambuf_iterator<char>());
            int asserts_in_clean = count_regex(clean_content, "\\bassert\\s*\\(") + count_regex(clean_content, "\\bcover\\s*\\(");
            if (asserts_in_clean == 0) {
                cout << "  [GUARD] Stage 3 produced 0 properties — restoring previous Stage 4 snapshot and skipping round.\n";
                ofstream restore(work_dir + "/stage4_results.json"); restore << s4_snapshot.dump(2);
                continue;
            }
        }

        bool stg4_pass = stage4(work_dir, module_name, rtl_code,
                                "stage_5_round_" + to_string(round));
        json s4now = load_json_file(work_dir + "/stage4_results.json");
        if (s4now.value("empty", false)) {
            cout << "  [GUARD] Stage 4 reported empty — restoring previous snapshot.\n";
            ofstream restore(work_dir + "/stage4_results.json"); restore << s4_snapshot.dump(2);
            continue;
        }
        int fail_after = (int)get_array(s4now, "failed").size();

        // Track cumulative best snapshot (by assert-pass count). Store FULL vault (incl. covers)
        // so rollback preserves cover coverage alongside assert coverage.
        {
            string tmp_asserts; set<string> tmp_lbl_a;
            int now_asserts = vault_rebuild_from_passed(tmp_asserts, tmp_lbl_a, get_array(s4now, "passed"), false);
            if (now_asserts > best_assert_passes) {
                string tmp_full; set<string> tmp_lbl_full;
                vault_rebuild_from_passed(tmp_full, tmp_lbl_full, get_array(s4now, "passed"), /*include_covers=*/true);
                best_assert_passes = now_asserts;
                best_locked_passes = tmp_full;
                best_vault_labels  = tmp_lbl_full;
                cout << "  [Best] New high-water mark: " << now_asserts << " passing asserts (vault stores " << tmp_lbl_full.size() << " props incl. covers)\n";
            }
        }

        if (stg4_pass) {
            cout << "  All assertions PASS after round " << round << "!\n";
            return json{{"status", "PASS"}, {"rounds", round}, {"vault_labels", vault_labels.size()}};
        }

        if (fail_after >= fail_before) {
            cout << "  No improvement (" << fail_after << " failures vs " << fail_before << " before round).\n";
            // Don't give up on first stall — keep trying with hotter temp. Break only on last round.
            if (round == max_rounds) {
                cout << "  Reached max_rounds; stopping.\n";
                break;
            }
        }
    }

    // =================================================================
    // FOCUSED SALVAGE PHASE — one targeted round per uncovered signal.
    // Motivation: broad Stage-5 rounds spray properties across all signals.
    // Signals whose initial asserts all failed BMC stay at 0 forever.
    // Here we ask the LLM for N asserts ABOUT ONE SIGNAL, then filter and BMC.
    // =================================================================
    {
        string cur_vault;
        set<string> cur_labels;
        vault_rebuild_from_passed(cur_vault, cur_labels,
                                  get_array(load_json_file(work_dir + "/stage4_results.json"), "passed"), false);
        vector<string> uncovered = compute_uncovered_signals(info_bank, cur_vault);
        if (!uncovered.empty()) {
            cout << "\n  --- Focused Salvage Phase: " << uncovered.size() << " uncovered signal(s) ---\n";
            for (const string& sig : uncovered) cout << "    - " << sig << "\n";

            string clk_name = get_str(info_bank, "clock", "clk");
            string rst_name = get_str(info_bank, "reset", "rst");
            string pol      = get_str(info_bank, "reset_polarity", "active_low");
            string rst_active_expr = (pol == "active_low") ? ("!" + rst_name) : rst_name;
            string preamble =
                "// ---- Stage 5 focused-salvage preamble ----\n"
                "reg f_past_valid;\ninitial f_past_valid = 0;\n"
                "always @(posedge " + clk_name + ") f_past_valid <= 1;\n"
                "reg [1:0] fmsv_init_cnt;\ninitial fmsv_init_cnt = 0;\n"
                "always @(posedge " + clk_name + ") if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;\n"
                "always @(*) if (fmsv_init_cnt < 2'd2) assume(" + rst_active_expr + ");\n\n";

            int focus_idx = 0;
            for (const string& sig : uncovered) {
                focus_idx++;
                cout << "\n  [Focus " << focus_idx << "/" << uncovered.size() << "] target=" << sig << "\n";
                string focus_prompt = build_focused_prompt(sig, info_bank, rtl_code, focus_idx);
                { ofstream p(work_dir + "/stage5_focus_" + sig + "_prompt.txt"); p << focus_prompt; }

                string llm_out;
                try {
                    llm_out = clean_assertions(clean_llm(client.chat(LEADER_SYSTEM, focus_prompt, "leader", 0.3f)));
                } catch (const exception& e) {
                    cout << "    [Focus] LLM error on " << sig << ": " << e.what() << " — skipping.\n";
                    continue;
                }
                { ofstream o(work_dir + "/stage5_focus_" + sig + "_raw.sv"); o << llm_out; }

                string filtered = filter_properties_mentioning(llm_out, sig);
                { ofstream o(work_dir + "/stage5_focus_" + sig + "_filtered.sv"); o << filtered; }
                if (trim(filtered).empty()) {
                    cout << "    [Focus] LLM produced nothing referencing " << sig << " — skipping.\n";
                    continue;
                }

                // Snapshot to restore on regression.
                json s4_snap = load_json_file(work_dir + "/stage4_results.json");

                // Rebuild current best vault fresh — INCLUDE covers for injection so Stage 6
                // preserves cover coverage (reachability) while still giving focus to new asserts.
                string vault_now;
                set<string> labels_now;
                vault_rebuild_from_passed(vault_now, labels_now,
                                          get_array(s4_snap, "passed"), /*include_covers=*/true);

                string vault_wrapped = wrap_bare_asserts(vault_now, clk_name);
                string combined = preamble + vault_wrapped
                                + "\n// ---- Focused salvage: " + sig + " ----\n"
                                + filtered;

                if (!stage3(client, rtl_code, combined, info_bank, work_dir)) {
                    cout << "    [Focus] Stage 3 failed — restoring snapshot.\n";
                    ofstream r(work_dir + "/stage4_results.json"); r << s4_snap.dump(2);
                    continue;
                }
                {
                    ifstream ck(work_dir + "/assertions_compiled_clean.sv");
                    string s((istreambuf_iterator<char>(ck)), istreambuf_iterator<char>());
                    int n = count_regex(s, "\\bassert\\s*\\(") + count_regex(s, "\\bcover\\s*\\(");
                    if (n == 0) {
                        cout << "    [Focus] Stage 3 stripped all — restoring.\n";
                        ofstream r(work_dir + "/stage4_results.json"); r << s4_snap.dump(2);
                        continue;
                    }
                }
                stage4(work_dir, module_name, rtl_code, "stage_5_salvage:" + sig);
                json s4_new = load_json_file(work_dir + "/stage4_results.json");
                if (s4_new.value("empty", false)) {
                    cout << "    [Focus] Stage 4 empty — restoring.\n";
                    ofstream r(work_dir + "/stage4_results.json"); r << s4_snap.dump(2);
                    continue;
                }

                // Check if this signal now has a passing assert.
                string new_vault; set<string> new_lbls;
                int new_cnt = vault_rebuild_from_passed(new_vault, new_lbls,
                                                       get_array(s4_new, "passed"), false);
                regex rx_sig("\\b" + sig + "\\b");
                bool sig_now_covered = regex_search(new_vault, rx_sig);
                bool improved = new_cnt > best_assert_passes;
                if (sig_now_covered && improved) {
                    // Store FULL vault (incl. covers) for rollback path.
                    string full_vault; set<string> full_lbls;
                    vault_rebuild_from_passed(full_vault, full_lbls,
                                              get_array(s4_new, "passed"), /*include_covers=*/true);
                    best_assert_passes = new_cnt;
                    best_locked_passes = full_vault;
                    best_vault_labels  = full_lbls;
                    cout << "    [Focus] +covered " << sig << " (asserts now " << new_cnt << ")\n";
                } else if (sig_now_covered) {
                    cout << "    [Focus] " << sig << " covered (no net increase — " << new_cnt << ")\n";
                } else {
                    cout << "    [Focus] " << sig << " still uncovered after BMC — restoring snapshot.\n";
                    ofstream r(work_dir + "/stage4_results.json"); r << s4_snap.dump(2);
                }
            }
        } else {
            cout << "\n  [Focus] All assertable signals already have >=1 passing assert — skipping salvage.\n";
        }
    }

    // Final report — prefer the cumulative best if it beats the last-round state.
    json last = load_json_file(work_dir + "/stage4_results.json");
    int remaining = (int)get_array(last, "failed").size();
    int last_asserts = 0;
    {
        string tmp; set<string> tmp_lbl;
        last_asserts = vault_rebuild_from_passed(tmp, tmp_lbl, get_array(last, "passed"), false);
    }
    if (best_assert_passes > last_asserts) {
        cout << "  [Best] Rolling back to cumulative best snapshot ("
             << best_assert_passes << " asserts > last round's " << last_asserts << ")\n";
        string clk_name = get_str(info_bank, "clock", "clk");
        string rst_name = get_str(info_bank, "reset", "rst");
        string pol      = get_str(info_bank, "reset_polarity", "active_low");
        string rst_active_expr = (pol == "active_low") ? ("!" + rst_name) : rst_name;
        string preamble =
            "reg f_past_valid;\ninitial f_past_valid = 0;\n"
            "always @(posedge " + clk_name + ") f_past_valid <= 1;\n"
            "reg [1:0] fmsv_init_cnt;\ninitial fmsv_init_cnt = 0;\n"
            "always @(posedge " + clk_name + ") if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;\n"
            "always @(*) if (fmsv_init_cnt < 2'd2) assume(" + rst_active_expr + ");\n\n";
        string combined = preamble + wrap_bare_asserts(best_locked_passes, clk_name);
        stage3(client, rtl_code, combined, info_bank, work_dir);
        stage4(work_dir, module_name, rtl_code, "stage_5_best_restore");
        last = load_json_file(work_dir + "/stage4_results.json");
        remaining = (int)get_array(last, "failed").size();
    }
    return json{{"status", "PARTIAL"}, {"rounds", max_rounds}, {"remaining_failures", remaining},
                {"vault_labels", best_vault_labels.size()}};
}

// =====================================================================
//  STAGE 6: VACUITY · SUBSUMPTION · CLUSTERING · CLASSIFICATION · REPORT
// =====================================================================

struct AssertionEntry {
    string label;
    string code;            // full Verilog line (label: assert/cover(...))
    string type;            // "assert" or "cover"
    string primary_signal;  // first signal that appears in the expression
    string classification;  // safety / liveness / reachability / protocol
    bool vacuous = false;
    bool subsumed = false;
    // Provenance from Stage 4 history (see stage4_history.json)
    string origin;              // e.g. "stage_4_initial", "stage_5_round_2", "stage_5_salvage:d_o"
    int    bmc_depth = 0;       // SBY-configured bound this property was run against
    int    proven_depth = 0;    // highest BMC step it was checked at (PASS held for all)
    int    first_proven_depth = 0;  // proven_depth when this label was FIRST proven
};

static string regex_escape_ident(const string& s) {
    string o;
    static const string specials = ".^$|()[]{}*+?\\";
    for (char c : s) {
        if (specials.find(c) != string::npos) o += '\\';
        o += c;
    }
    return o;
}

// Assertable (non-input) RTL names from Stage 1 information bank — longest first for primary-signal match.
static vector<string> get_assertable_signal_names(const json& info_bank) {
    vector<string> names;
    for (const auto& s : get_array(info_bank, "signals")) {
        if (get_str(s, "direction", "") == "input") continue;
        string n = get_str(s, "rtl_name", "");
        if (!n.empty()) names.push_back(n);
    }
    sort(names.begin(), names.end(), [](const string& a, const string& b) { return a.size() > b.size(); });
    return names;
}

static string primary_signal_from_code(const string& code, const vector<string>& assertable_sorted) {
    for (const string& name : assertable_sorted) {
        try {
            regex r("\\b" + regex_escape_ident(name) + "\\b");
            if (regex_search(code, r)) return name;
        } catch (...) { continue; }
    }
    return "unknown";
}

// Stage 6 input: ONLY mathematically proven lines from stage4_results.json "passed" array.
static vector<AssertionEntry> parse_passed_from_stage4(const string& work_dir, const vector<string>& assertable) {
    json s4 = load_json_file(work_dir + "/stage4_results.json");
    json passed = get_array(s4, "passed");
    vector<AssertionEntry> entries;
    regex rx_assert(R"(\bassert\s*\()");
    regex rx_cover(R"(\bcover\s*\()");
    regex rx_label_prefix(R"((\b[a-zA-Z0-9_]+)\s*:)");

    for (const auto& p : passed) {
        AssertionEntry e;
        if (p.is_object()) {
            e.code = trim(get_str(p, "original_code", ""));
            if (e.code.empty()) continue;
            e.label = get_str(p, "label", "");
            if (e.label.empty()) {
                smatch m;
                if (regex_search(e.code, m, rx_label_prefix)) e.label = m[1];
                else e.label = "unknown_label";
            }
            if (regex_search(e.code, rx_cover)) e.type = "cover";
            else if (regex_search(e.code, rx_assert)) e.type = "assert";
            else continue;
            // Provenance (may be absent on older stage4_results.json — defaults are 0/"")
            e.origin             = get_str(p, "origin", "");
            e.bmc_depth          = p.value("bmc_depth",          0);
            e.proven_depth       = p.value("proven_depth",       0);
            e.first_proven_depth = p.value("first_proven_depth", e.proven_depth);
        } else if (p.is_string()) {
            // Legacy: label only — cannot run vacuity/subsumption without code
            continue;
        } else {
            continue;
        }
        e.primary_signal = primary_signal_from_code(e.code, assertable);
        entries.push_back(e);
    }
    return entries;
}

// Formal coverage: split by property kind.
//  - assert_covered: signals referenced by a surviving ASSERT (semantic / correctness coverage)
//  - cover_covered:  signals referenced by a surviving COVER (reachability only — NOT correctness)
//  - any_covered:    union of both (kept for backward-compat reporting)
static void compute_formal_signal_coverage(const vector<AssertionEntry>& entries,
                                           const vector<string>& assertable,
                                           set<string>& assert_covered_out,
                                           set<string>& cover_covered_out,
                                           set<string>& any_covered_out,
                                           int& total_assertable_out,
                                           int& n_assert_covered_out,
                                           int& n_cover_covered_out,
                                           int& n_any_covered_out,
                                           double& pct_assert_out,
                                           double& pct_cover_out,
                                           double& pct_any_out) {
    total_assertable_out = (int)assertable.size();
    string assert_blob, cover_blob;
    for (const auto& e : entries) {
        if (e.vacuous || e.subsumed) continue;
        if (e.type == "assert") assert_blob += e.code + " ";
        else if (e.type == "cover") cover_blob += e.code + " ";
    }
    assert_covered_out.clear();
    cover_covered_out.clear();
    any_covered_out.clear();
    for (const string& name : assertable) {
        try {
            regex r("\\b" + regex_escape_ident(name) + "\\b");
            bool in_assert = regex_search(assert_blob, r);
            bool in_cover  = regex_search(cover_blob,  r);
            if (in_assert) assert_covered_out.insert(name);
            if (in_cover)  cover_covered_out.insert(name);
            if (in_assert || in_cover) any_covered_out.insert(name);
        } catch (...) { continue; }
    }
    n_assert_covered_out = (int)assert_covered_out.size();
    n_cover_covered_out  = (int)cover_covered_out.size();
    n_any_covered_out    = (int)any_covered_out.size();
    pct_assert_out = (total_assertable_out > 0) ? (100.0 * n_assert_covered_out / total_assertable_out) : 0.0;
    pct_cover_out  = (total_assertable_out > 0) ? (100.0 * n_cover_covered_out  / total_assertable_out) : 0.0;
    pct_any_out    = (total_assertable_out > 0) ? (100.0 * n_any_covered_out    / total_assertable_out) : 0.0;
}

static string classify_assertion(const AssertionEntry& e) {
    if (e.type == "cover") return "reachability";
    string c = e.code;
    if (c.find("$past") != string::npos && c.find("rst") != string::npos) return "safety";
    if (c.find("$stable") != string::npos || c.find("$rose") != string::npos || c.find("$fell") != string::npos) return "liveness";
    if (c.find("pstate") != string::npos || c.find("nstate") != string::npos) return "protocol";
    if (c.find("selection") != string::npos) return "protocol";
    return "safety";
}

json stage6(const string& work_dir, const string& module_name, const string& rtl_code, const json& info_bank) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "STAGE 6: VACUITY · SUBSUMPTION · CLUSTERING · REPORT\n";
    cout << string(60, '=') << "\n";

    if (!fs::exists(work_dir + "/stage4_results.json"))
        throw runtime_error("stage4_results.json not found — cannot run Stage 6.");

    vector<string> assertable_signals = get_assertable_signal_names(info_bank);
    cout << "  Assertable signals (from information bank): " << assertable_signals.size() << "\n";

    vector<AssertionEntry> entries = parse_passed_from_stage4(work_dir, assertable_signals);
    int initial_count = (int)entries.size();
    if (initial_count == 0)
        throw runtime_error("stage4_results.json has no passed entries with original_code — cannot run Stage 6.");
    cout << "  Loaded " << initial_count << " BMC-PASSED line(s) from stage4_results.json (passed[] only).\n";

    // ─── Step 1: VACUITY CHECK ───────────────────────────────────────
    cout << "\n  [6A] VACUITY CHECK\n";
    int vacuous_count = 0;
    string vacuity_dir = work_dir + "/stage6_vacuity";
    fs::create_directories(vacuity_dir);

    // 1a. TAUTOLOGY DETECTION — parse dut_with_asserts.sv to find each label's guard, then flag
    //     any assertion where the asserted expression (or its negation) is textually present
    //     as a conjunct of the guard. Example caught:
    //         if (... && d_o == 1'b0)  a_bad: assert(d_o == 1'b0);
    {
        string dut_with_asserts_path = work_dir + "/dut_with_asserts.sv";
        ifstream df(dut_with_asserts_path);
        if (df.is_open()) {
            string dut_full((istreambuf_iterator<char>(df)), istreambuf_iterator<char>());
            // For each entry label, search for "if (GUARD) LABEL: assert(EXPR);"
            // GUARD and EXPR captured with non-greedy paren-balanced best-effort regex.
            for (auto& e : entries) {
                if (e.type != "assert") continue;
                // Extract EXPR from e.code (the inner assert(...) expression).
                smatch em;
                regex rx_expr(R"(assert\s*\(\s*(.+?)\s*\)\s*;)");
                if (!regex_search(e.code, em, rx_expr)) continue;
                string inner_expr = trim(em[1].str());
                if (inner_expr.empty()) continue;

                // Find guard preceding "LABEL:" in the injected DUT.
                regex rx_block("if\\s*\\(([^)]*(?:\\([^)]*\\)[^)]*)*)\\)\\s*" + e.label + "\\s*:");
                smatch gm;
                if (!regex_search(dut_full, gm, rx_block)) continue;
                string guard = gm[1].str();
                if (guard.find(inner_expr) != string::npos) {
                    e.vacuous = true;
                    vacuous_count++;
                    cout << "    [TAUTOLOGY] " << e.label
                         << " — guard contains the asserted expression `" << inner_expr << "`\n";
                }
            }
        } else {
            cout << "    (skipping tautology check — dut_with_asserts.sv not readable)\n";
        }
    }

    cout << "  [6A.2] REACHABILITY-BASED VACUITY (precondition reachable?)\n";

    for (auto& e : entries) {
        if (e.type != "assert") continue;
        if (e.vacuous) continue;  // already flagged as tautology

        // e.code may be either bare (`label: assert(expr);`) OR already wrapped
        // (`always @(posedge clk) if (f_past_valid) label: assert(expr);`).
        // Strip any leading `always @(...) ... if (...) ` so we get just `label: assert(expr);`,
        // then convert to cover for reachability. Otherwise nested always blocks break vacuity.
        string inner = e.code;
        {
            smatch m;
            regex rx_strip(R"(^\s*always\s*@\([^)]*\)\s*(?:if\s*\([^)]*\)\s*)?)");
            inner = regex_replace(inner, rx_strip, "");
        }
        string cover_line = regex_replace(inner, regex(R"(\bassert\s*\()"), "cover(");
        cover_line = regex_replace(cover_line, regex(e.label + "\\s*:"), "vac_" + e.label + ":");

        string assert_block = "reg f_past_valid;\ninitial f_past_valid = 0;\n"
            "always @(posedge clk) f_past_valid <= 1;\n\n"
            "always @(posedge clk) begin\n"
            "    if (f_past_valid)\n"
            "        " + cover_line + "\n"
            "end\n";

        string injected = rtl_code;
        size_t pos = injected.rfind("endmodule");
        if (pos != string::npos) injected.insert(pos, "\n" + assert_block + "\n");

        string dut_file = vacuity_dir + "/vac_dut.sv";
        { ofstream o(dut_file); o << injected; }

        string sby =
            "[options]\nmode cover\ndepth 20\n\n[engines]\nsmtbmc z3\n\n"
            "[script]\nread_verilog -sv vac_dut.sv\nprep -top " + module_name + "\n\n"
            "[files]\nvac_dut.sv\n";
        { ofstream o(vacuity_dir + "/vac.sby"); o << sby; }

        string cmd = "export PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" && cd " + vacuity_dir + " && sby -f vac.sby > vac.log 2>&1";
        system(cmd.c_str());

        ifstream log_in(vacuity_dir + "/vac.log");
        string log_content((istreambuf_iterator<char>(log_in)), istreambuf_iterator<char>());

        if (log_content.find("DONE (PASS") != string::npos) {
            // Cover was reached → precondition is satisfiable → NOT vacuous
        } else {
            e.vacuous = true;
            vacuous_count++;
            cout << "    [VACUOUS] " << e.label << " — precondition unreachable\n";
        }
    }
    cout << "  Vacuity check complete: " << vacuous_count << " vacuous assertion(s) flagged.\n";

    // ─── Step 2: SUBSUMPTION ENGINE ──────────────────────────────────
    cout << "\n  [6B] SUBSUMPTION ENGINE (pairwise assume-A → assert-B)...\n";
    string subsump_dir = work_dir + "/stage6_subsumption";
    fs::create_directories(subsump_dir);

    // Collect non-vacuous assertions only
    vector<int> valid_assert_idx;
    for (int i = 0; i < (int)entries.size(); ++i)
        if (entries[i].type == "assert" && !entries[i].vacuous) valid_assert_idx.push_back(i);

    int subsume_count = 0;
    int pairs_tested = 0;

    for (int ai = 0; ai < (int)valid_assert_idx.size(); ++ai) {
        for (int bi = 0; bi < (int)valid_assert_idx.size(); ++bi) {
            if (ai == bi) continue;
            // Skip if A is already subsumed — prevents equivalence-class wipe-out where A≡B≡C
            // would leave all three marked subsumed (A subs B, then B subs C, then C subs A...).
            if (entries[valid_assert_idx[ai]].subsumed) continue;
            if (entries[valid_assert_idx[bi]].subsumed) continue;

            auto& eA = entries[valid_assert_idx[ai]];
            auto& eB = entries[valid_assert_idx[bi]];

            // If A and B target different signals, skip (heuristic prune)
            if (eA.primary_signal != eB.primary_signal) continue;

            pairs_tested++;

            // Build: assume(A_expr), assert(B_expr)
            // Extract just the expression from "label: assert(EXPR);"
            regex rx_expr(R"(assert\s*\((.+)\)\s*;?)");
            smatch mA, mB;
            if (!regex_search(eA.code, mA, rx_expr)) continue;
            if (!regex_search(eB.code, mB, rx_expr)) continue;

            string assume_line = "sub_assume: assume(" + mA[1].str() + ");";
            string assert_line = "sub_assert: assert(" + mB[1].str() + ");";

            string block = "reg f_past_valid;\ninitial f_past_valid = 0;\n"
                "always @(posedge clk) f_past_valid <= 1;\n\n"
                "always @(posedge clk) begin\n"
                "    if (f_past_valid) begin\n"
                "        " + assume_line + "\n"
                "        " + assert_line + "\n"
                "    end\n"
                "end\n";

            string injected = rtl_code;
            size_t pos = injected.rfind("endmodule");
            if (pos != string::npos) injected.insert(pos, "\n" + block + "\n");

            string dut_file = subsump_dir + "/sub_dut.sv";
            { ofstream o(dut_file); o << injected; }

            string sby =
                "[options]\nmode bmc\ndepth 10\n\n[engines]\nsmtbmc z3\n\n"
                "[script]\nread_verilog -sv sub_dut.sv\nprep -top " + module_name + "\n\n"
                "[files]\nsub_dut.sv\n";
            { ofstream o(subsump_dir + "/sub.sby"); o << sby; }

            string cmd = "export PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" && cd " + subsump_dir + " && sby -f sub.sby > sub.log 2>&1";
            system(cmd.c_str());

            ifstream log_in(subsump_dir + "/sub.log");
            string log_content((istreambuf_iterator<char>(log_in)), istreambuf_iterator<char>());

            if (log_content.find("DONE (PASS") != string::npos) {
                // A ⊃ B (A subsumes B) — B is redundant
                eB.subsumed = true;
                subsume_count++;
                cout << "    [SUBSUMED] " << eB.label << " is implied by " << eA.label << "\n";
            }
        }
    }
    cout << "  Subsumption complete: " << pairs_tested << " pairs tested, " << subsume_count << " redundant assertion(s) removed.\n";

    // ─── Step 3: CLASSIFICATION ──────────────────────────────────────
    cout << "\n  [6C] CLASSIFICATION (tagging property types)...\n";
    for (auto& e : entries) e.classification = classify_assertion(e);

    // ─── Step 4: CLUSTERING (group by primary signal) ────────────────
    cout << "  [6D] CLUSTERING (grouping by primary driving signal)...\n";
    map<string, vector<int>> clusters;
    for (int i = 0; i < (int)entries.size(); ++i) {
        if (!entries[i].vacuous && !entries[i].subsumed)
            clusters[entries[i].primary_signal].push_back(i);
    }

    // ─── Step 5: FINAL OUTPUT ────────────────────────────────────────
    cout << "\n  [6E] GENERATING FINAL ARTIFACTS...\n";

    // Count survivors (and kind breakdown) — used by report body AND summary.
    int survived = 0;
    int survived_assert_count = 0, survived_cover_count = 0;
    for (const auto& e : entries) {
        if (e.vacuous || e.subsumed) continue;
        survived++;
        if (e.type == "assert") survived_assert_count++;
        else if (e.type == "cover") survived_cover_count++;
    }

    set<string> assert_covered, cover_covered, any_covered;
    int total_assertable = 0;
    int n_assert_cov = 0, n_cover_cov = 0, n_any_cov = 0;
    double pct_assert = 0.0, pct_cover = 0.0, pct_any = 0.0;
    compute_formal_signal_coverage(entries, assertable_signals,
                                   assert_covered, cover_covered, any_covered,
                                   total_assertable,
                                   n_assert_cov, n_cover_cov, n_any_cov,
                                   pct_assert, pct_cover, pct_any);

    cout << "\n  [6F] FORMAL SIGNAL COVERAGE (assertable signals touched by surviving properties)\n";
    cout << "       Assert  (correctness):  " << n_assert_cov << " / " << total_assertable
         << " = " << fixed << setprecision(1) << pct_assert << "%\n";
    cout << "       Cover   (reachability): " << n_cover_cov << " / " << total_assertable
         << " = " << fixed << setprecision(1) << pct_cover << "%\n";
    cout << "       Any    (union):         " << n_any_cov << " / " << total_assertable
         << " = " << fixed << setprecision(1) << pct_any << "%\n";

    // Write final_fmsv_spec.sv
    string spec_path = work_dir + "/final_fmsv_spec.sv";
    {
        ofstream o(spec_path);
        o << "// ============================================================\n";
        o << "// FMSV Final Formal Specification — Auto-Generated\n";
        o << "// Source: stage4_results.json [passed] only (BMC-proven lines)\n";
        o << "// Initial passed: " << initial_count << " | Vacuous: " << vacuous_count
          << " | Subsumed: " << subsume_count << " | Final: " << survived << "\n";
        o << "// Assert coverage (correctness):  " << n_assert_cov << "/" << total_assertable
          << " assertable signals (" << fixed << setprecision(1) << pct_assert << "%)\n";
        o << "// Cover  coverage (reachability): " << n_cover_cov << "/" << total_assertable
          << " assertable signals (" << fixed << setprecision(1) << pct_cover << "%)\n";
        o << "// NOTE: cover() probes are reachability witnesses, not proofs of correctness.\n";
        o << "// ============================================================\n\n";

        o << "reg f_past_valid;\ninitial f_past_valid = 0;\n";
        o << "always @(posedge clk) f_past_valid <= 1;\n\n";

        for (const auto& [signal, idxs] : clusters) {
            o << "// --- Signal cluster: " << signal << " ---\n";
            for (int idx : idxs) {
                auto& e = entries[idx];
                o << "// [" << e.classification << "] " << e.label << "\n";
                if (e.type == "assert") {
                    o << "always @(posedge clk) begin\n";
                    o << "    if (f_past_valid)\n";
                    o << "        " << e.code << "\n";
                    o << "end\n";
                } else {
                    o << "always @(posedge clk)\n";
                    o << "    " << e.code << "\n";
                }
                o << "\n";
            }
        }
    }
    cout << "    Written: " << spec_path << "\n";

    // Write stage6_report.md
    string report_path = work_dir + "/stage6_report.md";
    {
        ofstream r(report_path);
        r << "# FMSV Stage 6 — Final Formal Verification Report\n\n";
        r << "## Summary\n\n";
        r << "| Metric | Count |\n";
        r << "|--------|-------|\n";
        r << "| Initial properties | " << initial_count << " |\n";
        r << "| Vacuous (removed) | " << vacuous_count << " |\n";
        r << "| Subsumed (redundant) | " << subsume_count << " |\n";
        r << "| **Final surviving** | **" << survived << "** |\n";
        r << "| Subsumption pairs tested | " << pairs_tested << " |\n\n";

        int n_assert = survived_assert_count, n_cover = survived_cover_count;
        int n_safety = 0, n_liveness = 0, n_reachability = 0, n_protocol = 0;
        for (const auto& e : entries) {
            if (e.vacuous || e.subsumed) continue;
            if (e.classification == "safety") n_safety++;
            else if (e.classification == "liveness") n_liveness++;
            else if (e.classification == "reachability") n_reachability++;
            else if (e.classification == "protocol") n_protocol++;
        }

        r << "## Property Breakdown\n\n";
        r << "| Type | Count |\n";
        r << "|------|-------|\n";
        r << "| Assertions | " << n_assert << " |\n";
        r << "| Covers | " << n_cover << " |\n\n";

        r << "## Classification\n\n";
        r << "| Class | Count |\n";
        r << "|-------|-------|\n";
        r << "| Safety | " << n_safety << " |\n";
        r << "| Liveness | " << n_liveness << " |\n";
        r << "| Reachability | " << n_reachability << " |\n";
        r << "| Protocol | " << n_protocol << " |\n\n";

        float retention_pct = initial_count > 0 ? ((float)survived / initial_count) * 100.0f : 0.0f;

        r << "## Formal signal coverage\n\n";
        r << "Assertable (non-input) signals from the information bank: **" << total_assertable << "**\n\n";
        r << "Coverage is reported **separately** for asserts (correctness) and covers (reachability). "
          << "A `cover` passing means the state is reachable — it is **not** a proof that the design behaves correctly. "
          << "Only `assert` coverage reflects verified functional properties.\n\n";
        r << "| Metric | Signals | Percent | Meaning |\n";
        r << "|--------|---------|---------|---------|\n";
        r << "| **Assert coverage (correctness)** | **" << n_assert_cov << " / " << total_assertable << "** | **"
          << fixed << setprecision(1) << pct_assert << "%** | Signals with a BMC-proven assertion |\n";
        r << "| Cover coverage (reachability only) | " << n_cover_cov << " / " << total_assertable << " | "
          << fixed << setprecision(1) << pct_cover << "% | Signals with a reachable `cover()` witness |\n";
        r << "| Union (any surviving property) | " << n_any_cov << " / " << total_assertable << " | "
          << fixed << setprecision(1) << pct_any << "% | Signals appearing in *any* surviving property |\n\n";

        if (n_assert == 0) {
            r << "> **WARNING:** Zero surviving `assert` properties. No functional correctness has been formally verified on this design. "
              << "All surviving properties are `cover` reachability probes. Assert coverage is **0%** regardless of the union figure.\n\n";
        } else if (n_assert_cov < total_assertable) {
            r << "> **NOTE:** Some assertable signals have no surviving `assert` — only `cover` witnesses. Those signals are not formally verified for correctness.\n\n";
        }

        r << "### Signals with surviving asserts (correctness-verified)\n\n";
        if (assert_covered.empty()) r << "None.\n";
        else for (const auto& s : assert_covered) r << "- `" << s << "`\n";
        r << "\n### Signals with only cover witnesses (reachability only)\n\n";
        {
            bool any = false;
            for (const auto& name : assertable_signals) {
                if (cover_covered.count(name) && !assert_covered.count(name)) { r << "- `" << name << "`\n"; any = true; }
            }
            if (!any) r << "None.\n";
        }
        r << "\n### Assertable signals not referenced in any surviving property\n\n";
        {
            bool any = false;
            for (const auto& name : assertable_signals) {
                if (!any_covered.count(name)) { r << "- `" << name << "`\n"; any = true; }
            }
            if (!any) r << "None.\n";
        }
        r << "\n";

        r << "## Signal Clusters\n\n";
        for (const auto& [signal, idxs] : clusters) {
            r << "### `" << signal << "` (" << idxs.size() << " properties)\n\n";
            for (int idx : idxs) {
                auto& e = entries[idx];
                r << "- **" << e.label << "** [" << e.classification << "] `" << e.code << "`\n";
            }
            r << "\n";
        }

        r << "## Vacuous Properties (Removed)\n\n";
        bool any_vac = false;
        for (const auto& e : entries) {
            if (e.vacuous) { r << "- ~~" << e.label << "~~ — `" << e.code << "`\n"; any_vac = true; }
        }
        if (!any_vac) r << "None.\n";
        r << "\n";

        r << "## Subsumed Properties (Removed)\n\n";
        bool any_sub = false;
        for (const auto& e : entries) {
            if (e.subsumed) { r << "- ~~" << e.label << "~~ — `" << e.code << "`\n"; any_sub = true; }
        }
        if (!any_sub) r << "None.\n";
        r << "\n";

        r << "## Property retention (optimization)\n\n";
        r << "**" << survived << " / " << initial_count << "** BMC-passed properties survived vacuity/subsumption = **"
          << fixed << setprecision(1) << retention_pct << "%** retention\n\n";

        // ── Pass-provenance breakdown — proves NON-vacuous passing + where each came from ──
        r << "## Pass Provenance & BMC Depth\n\n";
        r << "Each property below was bounded-model-checked by SymbiYosys with `mode bmc, depth 20`. "
          << "A `proven_depth` of N means the property held for every cycle `0..N-1` of the BMC trace — "
          << "a proof at depth ≥ 2 is evidence the property is **not a Step-0 / reset-vector tautology**.\n\n";

        // Tally input totals per origin (ALL initially-passed, pre-vacuity).
        map<string, int> origin_total;
        map<string, int> origin_survived;
        map<string, int> origin_vacuous;
        map<string, int> origin_subsumed;
        int depth_ge2 = 0, depth_lt2 = 0, depth_unknown = 0;
        int depth_ge2_surv = 0;
        for (const auto& e : entries) {
            string o = e.origin.empty() ? "unknown" : e.origin;
            origin_total[o]++;
            if (e.vacuous)  origin_vacuous[o]++;
            if (e.subsumed) origin_subsumed[o]++;
            if (!e.vacuous && !e.subsumed) origin_survived[o]++;

            if (e.proven_depth >= 2)      { depth_ge2++; if (!e.vacuous && !e.subsumed) depth_ge2_surv++; }
            else if (e.proven_depth >= 1) depth_lt2++;
            else                          depth_unknown++;
        }

        r << "### Totals by BMC proof depth (all " << initial_count << " initially-passed)\n\n";
        r << "| Proven depth | Count | Interpretation |\n";
        r << "|---|---|---|\n";
        r << "| ≥ 2 cycles | " << depth_ge2 << " | Non-trivial: survived beyond Step 0 / reset-hold |\n";
        r << "| = 1 cycle  | " << depth_lt2 << " | Only held at Step 0 — likely vacuous/reset-only |\n";
        r << "| unknown / 0 | " << depth_unknown << " | Depth not captured (legacy entries) |\n\n";
        r << "Of the **" << survived << "** survivors after vacuity + subsumption, "
          << "**" << depth_ge2_surv << "** were proven at depth ≥ 2 cycles.\n\n";

        r << "### Properties first proven per stage/round\n\n";
        r << "This shows where in the pipeline each surviving property was *first* proven. "
          << "`stage_4_initial` = proven on the very first Stage-2 LLM batch (before any refinement).\n\n";
        r << "| Origin | First-passed | Survived Stage 6 | Vacuous | Subsumed |\n";
        r << "|---|---|---|---|---|\n";
        // Ordered listing: initial → rounds → salvage → rest
        vector<string> origin_order;
        origin_order.push_back("stage_4_initial");
        for (int rr = 1; rr <= 4; ++rr) origin_order.push_back("stage_5_round_" + to_string(rr));
        // Salvage/other tags: append any not already listed, sorted.
        set<string> seen(origin_order.begin(), origin_order.end());
        vector<string> extras;
        for (const auto& kv : origin_total) if (!seen.count(kv.first)) extras.push_back(kv.first);
        sort(extras.begin(), extras.end());
        for (const auto& o : extras) origin_order.push_back(o);

        int cumulative_first = 0, cumulative_survived = 0;
        for (const auto& o : origin_order) {
            if (!origin_total.count(o)) continue;
            cumulative_first    += origin_total[o];
            cumulative_survived += origin_survived[o];
            r << "| `" << o << "` | " << origin_total[o] << " | " << origin_survived[o]
              << " | " << origin_vacuous[o] << " | " << origin_subsumed[o] << " |\n";
        }
        r << "| **Cumulative total** | **" << cumulative_first << "** | **" << cumulative_survived
          << "** | — | — |\n\n";

        r << "> **Cumulative total passed = " << cumulative_first << "** (sum of all BMC-passed properties across "
          << "Stage 4 initial + every Stage 5 refinement round + focused salvage).\n"
          << "> **Final surviving after Stage 6 (vacuity + subsumption + clustering) = " << survived << "**.\n\n";

        r << "### Per-property provenance\n\n";
        r << "| Label | Type | Origin | Proven depth | Status |\n";
        r << "|---|---|---|---|---|\n";
        for (const auto& e : entries) {
            string status = "survived";
            if (e.vacuous)  status = "vacuous";
            else if (e.subsumed) status = "subsumed";
            string origin_disp = e.origin.empty() ? "unknown" : e.origin;
            string depth_disp  = (e.proven_depth > 0) ? (to_string(e.proven_depth) + " cycles") : "—";
            r << "| `" << e.label << "` | " << e.type << " | `" << origin_disp << "` | "
              << depth_disp << " | " << status << " |\n";
        }
        r << "\n";

        r << "---\n*Generated by FMSV Pipeline v4 — Stage 6*\n";
    }
    cout << "    Written: " << report_path << "\n";

    // Write stage6_results.json
    json results;
    results["initial_count"] = initial_count;
    results["vacuous_removed"] = vacuous_count;
    results["subsumed_removed"] = subsume_count;
    results["final_count"] = survived;
    results["final_assert_count"] = survived_assert_count;
    results["final_cover_count"] = survived_cover_count;
    results["pairs_tested"] = pairs_tested;
    results["assertable_signal_count"] = total_assertable;
    // Legacy fields (kept for backward compatibility) — "any surviving property" union.
    results["signals_covered_count"] = n_any_cov;
    results["formal_coverage_percent"] = round(pct_any * 10.0) / 10.0;
    // New split fields — only assert_coverage_percent reflects correctness.
    results["assert_signals_covered_count"] = n_assert_cov;
    results["assert_coverage_percent"] = round(pct_assert * 10.0) / 10.0;
    results["cover_signals_covered_count"] = n_cover_cov;
    results["cover_coverage_percent"] = round(pct_cover * 10.0) / 10.0;
    results["coverage_note"] = "assert_coverage_percent = correctness (BMC-proven asserts). "
                               "cover_coverage_percent = reachability witnesses only, NOT correctness. "
                               "formal_coverage_percent is the union and kept for backward compatibility.";
    json cov_arr = json::array();
    for (const auto& s : any_covered) cov_arr.push_back(s);
    results["covered_assertable_signals"] = cov_arr;
    json assert_cov_arr = json::array();
    for (const auto& s : assert_covered) assert_cov_arr.push_back(s);
    results["assert_covered_signals"] = assert_cov_arr;
    json cover_cov_arr = json::array();
    for (const auto& s : cover_covered) cover_cov_arr.push_back(s);
    results["cover_covered_signals"] = cover_cov_arr;

    // Provenance aggregates (cumulative across all stages/rounds before Stage-6 pruning)
    {
        map<string, int> o_total, o_surv, o_vac, o_sub;
        int d_ge2 = 0, d_lt2 = 0, d_unk = 0, d_ge2_surv = 0;
        for (const auto& e : entries) {
            string o = e.origin.empty() ? "unknown" : e.origin;
            o_total[o]++;
            if (e.vacuous) o_vac[o]++;
            if (e.subsumed) o_sub[o]++;
            if (!e.vacuous && !e.subsumed) o_surv[o]++;
            if (e.proven_depth >= 2)      { d_ge2++; if (!e.vacuous && !e.subsumed) d_ge2_surv++; }
            else if (e.proven_depth >= 1) d_lt2++;
            else                          d_unk++;
        }
        json prov = json::object();
        for (const auto& kv : o_total) {
            prov[kv.first] = {
                {"first_passed", kv.second},
                {"survived_stage6", o_surv[kv.first]},
                {"vacuous", o_vac[kv.first]},
                {"subsumed", o_sub[kv.first]}
            };
        }
        results["provenance_by_origin"] = prov;
        results["cumulative_passed_all_stages"] = initial_count;  // sum of every first-pass across all origins
        results["depth_breakdown"] = {
            {"proven_depth_ge2",      d_ge2},
            {"proven_depth_eq1",      d_lt2},
            {"proven_depth_unknown",  d_unk},
            {"survived_with_depth_ge2", d_ge2_surv}
        };
    }

    json surviving = json::array();
    for (const auto& e : entries) {
        if (e.vacuous || e.subsumed) continue;
        surviving.push_back({
            {"label", e.label}, {"type", e.type},
            {"classification", e.classification},
            {"signal", e.primary_signal}, {"code", e.code},
            {"origin", e.origin},
            {"bmc_depth", e.bmc_depth},
            {"proven_depth", e.proven_depth},
            {"first_proven_depth", e.first_proven_depth}
        });
    }
    results["surviving_properties"] = surviving;

    json vacuous_list = json::array();
    for (const auto& e : entries)
        if (e.vacuous) vacuous_list.push_back({{"label", e.label}, {"code", e.code}});
    results["vacuous"] = vacuous_list;

    json subsumed_list = json::array();
    for (const auto& e : entries)
        if (e.subsumed) subsumed_list.push_back({{"label", e.label}, {"code", e.code}});
    results["subsumed"] = subsumed_list;

    { ofstream o(work_dir + "/stage6_results.json"); o << results.dump(2); }
    cout << "    Written: " << work_dir << "/stage6_results.json\n";

    // Summary
    cout << "\n  " << string(40, '=') << "\n";
    cout << "  STAGE 6 SUMMARY\n";
    cout << "  " << string(40, '=') << "\n";
    cout << "  Initial properties:   " << initial_count << "\n";
    cout << "  Vacuous removed:      " << vacuous_count << "\n";
    cout << "  Subsumed removed:     " << subsume_count << "\n";
    cout << "  Final surviving:      " << survived << "\n";
    cout << "  Clusters:             " << clusters.size() << "\n";
    // Provenance summary (cumulative pass counts per stage/round)
    {
        map<string, int> o_total;
        int d_ge2 = 0, d_ge2_surv = 0;
        for (const auto& e : entries) {
            string o = e.origin.empty() ? "unknown" : e.origin;
            o_total[o]++;
            if (e.proven_depth >= 2) {
                d_ge2++;
                if (!e.vacuous && !e.subsumed) d_ge2_surv++;
            }
        }
        cout << "  --- Cumulative pass provenance ---\n";
        vector<string> order = {"stage_4_initial", "stage_5_round_1", "stage_5_round_2",
                                "stage_5_round_3", "stage_5_round_4", "stage_5_best_restore"};
        int sum = 0;
        for (const auto& o : order) {
            if (!o_total.count(o)) continue;
            cout << "    " << o << ": " << o_total[o] << " first-proven\n";
            sum += o_total[o];
        }
        int salvage_sum = 0;
        for (const auto& kv : o_total) {
            if (kv.first.rfind("stage_5_salvage", 0) == 0) salvage_sum += kv.second;
        }
        if (salvage_sum > 0) {
            cout << "    stage_5_salvage (all signals): " << salvage_sum << " first-proven\n";
            sum += salvage_sum;
        }
        for (const auto& kv : o_total) {
            bool listed = false;
            for (const auto& o : order) if (o == kv.first) { listed = true; break; }
            if (listed) continue;
            if (kv.first.rfind("stage_5_salvage", 0) == 0) continue;
            cout << "    " << kv.first << ": " << kv.second << " first-proven\n";
            sum += kv.second;
        }
        cout << "    Cumulative total passed across all stages: " << sum << "\n";
        cout << "    Proven at depth >= 2 cycles: " << d_ge2
             << " (of which " << d_ge2_surv << " survived Stage 6 pruning)\n";
    }
    cout << "  Assert  coverage:     " << fixed << setprecision(1) << pct_assert << "% ("
         << n_assert_cov << "/" << total_assertable << " signals — correctness)\n";
    cout << "  Cover   coverage:     " << fixed << setprecision(1) << pct_cover << "% ("
         << n_cover_cov << "/" << total_assertable << " signals — reachability only)\n";
    cout << "  Union   coverage:     " << fixed << setprecision(1) << pct_any << "% ("
         << n_any_cov << "/" << total_assertable << " signals)\n";
    if (survived_assert_count == 0)
        cout << "  *** WARNING: 0 surviving asserts — NO correctness formally verified ***\n";
    cout << "  Formal spec:          " << spec_path << "\n";
    cout << "  Report:               " << report_path << "\n";

    return results;
}

int main(int argc, char* argv[]) {
    string design_name, design_dir, genben = "../GenBen", config = "config.json";
    int start_stage = 1; // Default to running everything

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--design" && i + 1 < argc) design_name = argv[++i];
        else if (arg == "--design-dir" && i + 1 < argc) design_dir = argv[++i];
        else if (arg == "--genben" && i + 1 < argc) genben = argv[++i];
        else if (arg == "--config" && i + 1 < argc) config = argv[++i];
        else if (arg == "--stage" && i + 1 < argc) start_stage = stoi(argv[++i]);
    }

    if (design_dir.empty() && !design_name.empty()) design_dir = genben + "/data/Design/" + design_name;
    if (design_dir.empty()) { cerr << "ERROR: Provide --design or --design-dir\n"; return 1; }
    if (config == "config.json" && !fs::exists(config) && fs::exists("../config.json")) config = "../config.json";

    string rtl_path = design_dir + "/dut.v", spec_path = design_dir + "/spec.txt";
    if (!fs::exists(rtl_path) || !fs::exists(spec_path)) { cerr << "ERROR: Missing files in " << design_dir << "\n"; return 1; }

    ifstream rtl_f(rtl_path), spec_f(spec_path);
    string rtl_code((istreambuf_iterator<char>(rtl_f)), istreambuf_iterator<char>());
    string spec_text((istreambuf_iterator<char>(spec_f)), istreambuf_iterator<char>());

    string design_id = fs::path(design_dir).filename().string();
    string work_dir = "../run_" + design_id;
    fs::create_directories(work_dir);

    cout << "╔" << repeat_str("═", 58) << "╗\n";
    cout << "║  FMSV PIPELINE v4 (C++)                                ║\n";
    cout << "╚" << repeat_str("═", 58) << "╝\n";
    cout << "Design:  " << design_id << "\nWork:    " << work_dir << "\n";

    try {
        bool stg3_pass = false;
        string module_name;
        json info_bank;
        unique_ptr<GroqClient> groq;

        // Run LLM Stages (1-3) only if starting from stage 1, 2, or 3
        if (start_stage <= 3) {
            groq = make_unique<GroqClient>(config);
            info_bank = stage1(*groq, rtl_code, spec_text, work_dir);
            string assertion_code = stage2(*groq, info_bank, rtl_code, work_dir);
            stg3_pass = stage3(*groq, rtl_code, assertion_code, info_bank, work_dir);
            module_name = get_str(info_bank, "module_name", "top");
        } else {
            cout << "\n[RESUME] Skipping Stages 1-3. Loading existing data from " << work_dir << "...\n";
            ifstream ib_file(work_dir + "/information_bank.json");
            if (!ib_file.is_open()) throw runtime_error("information_bank.json not found! Cannot resume.");
            info_bank = json::parse(ib_file);
            module_name = get_str(info_bank, "module_name", "top");
            stg3_pass = true; // Assume previous run compiled successfully
        }

        // Run Stage 4, or reuse on-disk results when resuming at Stage 5 only
        bool stg4_pass = false;
        if (start_stage <= 4 && stg3_pass) {
            stg4_pass = stage4(work_dir, module_name, rtl_code);
        } else if (stg3_pass && start_stage >= 5) {
            if (!fs::exists(work_dir + "/stage4_results.json"))
                throw runtime_error("stage4_results.json not found in " + work_dir + " — run with --stage 4 first.");
            if (!fs::exists(work_dir + "/assertions_compiled_clean.sv"))
                throw runtime_error("assertions_compiled_clean.sv not found — run Stages 1-3 or --stage 4 first.");
            json s4_resume = load_json_file(work_dir + "/stage4_results.json");
            stg4_pass = get_array(s4_resume, "failed").empty();
            // Don't print "Stage 5 entry" when user only asked for --stage 6 (misleading).
            if (start_stage <= 5) {
                cout << "\n[RESUME] Stage 5 entry: using existing Stage 4 results — "
                     << (stg4_pass ? "all properties PASS (Stage 5 will skip LLM)." : "failures present — running refinement.") << "\n";
            } else if (start_stage == 6) {
                cout << "\n[RESUME] Stage 6 only — loaded stage4_results.json ("
                     << (stg4_pass ? "all properties PASS" : "some failures remain — Stage 6 will use passed[] only") << ").\n";
            }
        }

        json stage5_log;
        bool ran_stage5 = false;
        if (start_stage <= 5 && stg3_pass && !stg4_pass) {
            if (!groq) groq = make_unique<GroqClient>(config);
            stage5_log = stage5(*groq, work_dir, rtl_code, module_name, info_bank, 4);
            ran_stage5 = true;
            ofstream s5o(work_dir + "/stage5_refinement.json");
            s5o << stage5_log.dump(2);
            s5o.close();
            try {
                json s4 = load_json_file(work_dir + "/stage4_results.json");
                stg4_pass = get_array(s4, "failed").empty();
            } catch (...) {
                // keep stg4_pass as false
            }
        }

        // Stage 6: always runs after Stage 4 results exist (same run), using passed[] only — even if failed[] is non-empty.
        // Best case: all Stage 4 checks pass; otherwise we still optimize the BMC-proven subset.
        json stage6_log;
        bool ran_stage6 = false;
        const bool have_s4 = fs::exists(work_dir + "/stage4_results.json");
        bool run_stage6_now = (start_stage <= 6) && stg3_pass && have_s4;
        if (run_stage6_now) {
            stage6_log = stage6(work_dir, module_name, rtl_code, info_bank);
            ran_stage6 = true;
            ofstream s6o(work_dir + "/stage6_results.json");
            s6o << stage6_log.dump(2);
            s6o.close();
        }

        cout << "\n╔" << repeat_str("═", 58) << "╗\n";
        cout << "║  PIPELINE COMPLETE                                     ║\n";
        cout << "╚" << repeat_str("═", 58) << "╝\n";
        if (start_stage <= 3) cout << "Stage 3 Compilation: " << (stg3_pass ? "PASS" : "FAIL") << "\n";
        cout << "Stage 4 Formal Check: " << (stg4_pass ? "ALL PASSED" : "FAILURES DETECTED") << "\n";
        if (ran_stage5)
            cout << "Stage 5 Refinement:  " << get_str(stage5_log, "status", "?")
                 << " (rounds " << get_str(stage5_log, "rounds", "?") << ")\n";
        if (ran_stage6) {
            int final_n = stage6_log.value("final_count", 0);
            int init_n = stage6_log.value("initial_count", 0);
            int vac_n = stage6_log.value("vacuous_removed", 0);
            int sub_n = stage6_log.value("subsumed_removed", 0);
            int sc = stage6_log.value("signals_covered_count", 0);
            int sa = stage6_log.value("assertable_signal_count", 0);
            double fcp = stage6_log.value("formal_coverage_percent", 0.0);
            cout << "Stage 6 Optimization: " << init_n << " → " << final_n
                 << " (vacuous: " << vac_n << ", subsumed: " << sub_n << ")\n";
            cout << "  Formal coverage: " << fixed << setprecision(1) << fcp << "% (" << sc << "/" << sa << " assertable signals)\n";
            cout << "  Final spec: " << work_dir << "/final_fmsv_spec.sv\n";
            cout << "  Report:     " << work_dir << "/stage6_report.md\n";
            if (!stg4_pass)
                cout << "  (Stage 6 used stage4 passed[] only; full Stage 4 still shows failures.)\n";
        }

    } catch (const exception& e) {
        cerr << "\nFATAL ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}