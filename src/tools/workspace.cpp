// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.



#include "tools/workspace.h"
#include <string> //allowed for find and parsing stuff



namespace fs = std::filesystem;
//im not going to change the path provided, just uncommented
//checks also for unkown error as the last catch statement

//SPECS ORDER OF CHECKS
//1. empty 
//2. nul
//3. abcolute
//4. normalize
//5. ..
//6. return root or root/n

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
try { 
    fs::path p{std::string(user_path)}; //shortening
    //empty path is invalid check
    if (user_path.empty()) {
        *reason = "empty path"; //message out
        return false; //autofilled euer_path, check names
    }

    //nul character check
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "null char in path"; //message out
        return false; //autofilled user_path, check names
    }

    if (p.is_absolute() || p.has_root_path()) {
        *reason = "absolute path not allowed"; //message out
        return false; //autofilled user_path, check names
    }

     fs::path n = p.lexically_normal(); //normalize the path
    //tool that fixes . and .., and removes redundant slashes

    if (!n.empty() && *n.begin() == "..") {
        //this happens when a path starts to creep into other directories
        *reason = "path out of bounds"; //message out
        return false; //autofilled user_path, check names
    }

    
    if (n.empty() || n == ".") {
        *out = root(); //RETURN THE ROOT PATH, NOT THE USER PATH
    }

    else {
        *out = root() / n; //RETURN THE ROOT PATH, NOT THE USER PATH
    }
    return true; //if all checks pass, return true
}
catch (const std::exception& e) {
    *reason = std::string("exception: ") + e.what(); //message out directions

    return false; //fullfilled
    //*reason = "Workspace::resolve not implemented yet";   // TODO
   // return false;
}
catch (...) {
    *reason = "???"; //message out 
    return false; //fullfilled in case of unkown reason
}
}
