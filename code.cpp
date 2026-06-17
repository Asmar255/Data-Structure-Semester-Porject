

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// ── String Utilities ────────────────────────────────────────────────────────
// Manual implementations because <algorithm> / <cstring> are not allowed.

bool isSameString(string a, string b) {
    if (a.length() != b.length()) return false;
    for (int i = 0; i < (int)a.length(); i++)
        if (a[i] != b[i]) return false;
    return true;
}

// Returns -1, 0, or 1 — used for BST left/right decisions.
int compareStrings(string a, string b) {
    int shorter = (int)a.length() < (int)b.length() ? (int)a.length() : (int)b.length();
    for (int i = 0; i < shorter; i++) {
        if (a[i] < b[i]) return -1;
        if (a[i] > b[i]) return  1;
    }
    if (a.length() < b.length()) return -1;
    if (a.length() > b.length()) return  1;
    return 0;
}

// Converts a positive integer to string — needed for undo action tokens.
string intToString(int n) {
    if (n == 0) return "0";
    string result = "";
    while (n > 0) {
        result = (char)('0' + n % 10) + result;
        n /= 10;
    }
    return result;
}

// Converts a numeric string back to int — used when parsing saved post IDs.
int stringToInt(string s) {
    int result = 0;
    for (int i = 0; i < (int)s.length(); i++)
        result = result * 10 + (s[i] - '0');
    return result;
}


// ── Post (Doubly Linked List node) ──────────────────────────────────────────

struct Post {
    int    id;
    string text;
    string authorUsername;
    Post* prev;
    Post* next;

    Post(int id, string text, string author) {
        this->id = id;
        this->text = text;
        this->authorUsername = author;
        this->prev = NULL;
        this->next = NULL;
    }
};


// ── Connection (Friend entry in a Doubly Linked List) ───────────────────────

struct Connection {
    string      friendUsername;
    Connection* prev;
    Connection* next;

    Connection(string username) {
        this->friendUsername = username;
        this->prev = NULL;
        this->next = NULL;
    }
};


// ── ActionRecord (node for the undo stack) ──────────────────────────────────
// Stores a reversible action as a token string, e.g. "ADD_FRIEND:sara"

struct ActionRecord {
    string        token;
    ActionRecord* below; // points to the action beneath this one on the stack

    ActionRecord(string token) {
        this->token = token;
        this->below = NULL;
    }
};


// ── UndoStack ────────────────────────────────────────────────────────────────
// LIFO stack — each user gets one to track their last reversible actions.

struct UndoStack {
    ActionRecord* top;

    UndoStack() { top = NULL; }

    void push(string token) {
        ActionRecord* record = new ActionRecord(token);
        record->below = top;
        top = record;
    }

    // Returns the token and removes the top record. Returns "" if empty.
    string pop() {
        if (top == NULL) return "";
        string token = top->token;
        ActionRecord* old = top;
        top = top->below;
        delete old;
        return token;
    }

    bool isEmpty() { return top == NULL; }

    ~UndoStack() {
        while (!isEmpty()) pop();
    }
};


// ── FeedEntry (node for the global chronological feed queue) ─────────────────

struct FeedEntry {
    Post* post;
    FeedEntry* next;

    FeedEntry(Post* post) {
        this->post = post;
        this->next = NULL;
    }
};


// ── GlobalFeed ───────────────────────────────────────────────────────────────
// FIFO queue — new posts join at the rear; display reads from the front.

struct GlobalFeed {
    FeedEntry* front;
    FeedEntry* rear;

    GlobalFeed() {
        front = NULL;
        rear = NULL;
    }

    void addPost(Post* post) {
        FeedEntry* entry = new FeedEntry(post);
        if (rear == NULL) {
            front = rear = entry;
        }
        else {
            rear->next = entry;
            rear = entry;
        }
    }

    // Scans the queue and unlinks the entry whose post ID matches.
    void removePost(int postId) {
        FeedEntry* current = front;
        FeedEntry* previous = NULL;
        while (current != NULL) {
            if (current->post != NULL && current->post->id == postId) {
                if (previous == NULL) front = current->next;
                else                  previous->next = current->next;
                if (current == rear)  rear = previous;
                delete current;
                return;
            }
            previous = current;
            current = current->next;
        }
    }

    void display() {
        if (front == NULL) {
            cout << "  [Feed is empty]\n";
            return;
        }
        FeedEntry* current = front;
        while (current != NULL) {
            if (current->post != NULL) {
                cout << "  [#" << current->post->id << "] "
                    << "@" << current->post->authorUsername << ": "
                    << current->post->text << "\n";
            }
            current = current->next;
        }
    }

    ~GlobalFeed() {
        FeedEntry* current = front;
        while (current != NULL) {
            FeedEntry* next = current->next;
            delete current;
            current = next;
        }
    }
};


// ── User ─────────────────────────────────────────────────────────────────────
// Owns a post list (DLL), a connections list (DLL), and a personal undo stack.

struct User {
    string username;
    string password;

    Post* firstPost;
    Post* lastPost;
    int         totalPosts;

    Connection* firstConnection;
    Connection* lastConnection;
    int         totalConnections;

    UndoStack   actionHistory;

    User() {
        firstPost = NULL;
        lastPost = NULL;
        totalPosts = 0;
        firstConnection = NULL;
        lastConnection = NULL;
        totalConnections = 0;
    }

    // ── Post operations ─────────────────────────────────────────────────────

    void createPost(int id, string text, GlobalFeed& feed) {
        Post* post = new Post(id, text, username);

        if (firstPost == NULL) {
            firstPost = lastPost = post;
        }
        else {
            post->prev = lastPost;
            lastPost->next = post;
            lastPost = post;
        }

        totalPosts++;
        feed.addPost(post);
        actionHistory.push("CREATE_POST:" + intToString(id));
        cout << "  Post published. (ID: " << id << ")\n";
    }

    // Unlinks and deletes the post from both the user list and the global feed.
    void removePost(int id, GlobalFeed& feed) {
        Post* current = firstPost;
        while (current != NULL) {
            if (current->id == id) {
                if (current->prev != NULL) current->prev->next = current->next;
                else                       firstPost = current->next;
                if (current->next != NULL) current->next->prev = current->prev;
                else                       lastPost = current->prev;

                feed.removePost(id);
                totalPosts--;
                delete current;
                cout << "  Post #" << id << " removed.\n";
                return;
            }
            current = current->next;
        }
        cout << "  Post #" << id << " not found.\n";
    }

    void displayPosts() {
        if (firstPost == NULL) {
            cout << "  No posts yet.\n";
            return;
        }
        Post* current = firstPost;
        while (current != NULL) {
            cout << "  [#" << current->id << "] " << current->text << "\n";
            current = current->next;
        }
    }

    // ── Connection (friend) operations ──────────────────────────────────────

    bool isConnectedTo(string targetUsername) {
        Connection* current = firstConnection;
        while (current != NULL) {
            if (isSameString(current->friendUsername, targetUsername)) return true;
            current = current->next;
        }
        return false;
    }

    void addConnection(string targetUsername) {
        if (isConnectedTo(targetUsername)) {
            cout << "  Already connected with @" << targetUsername << ".\n";
            return;
        }
        Connection* link = new Connection(targetUsername);
        if (firstConnection == NULL) {
            firstConnection = lastConnection = link;
        }
        else {
            link->prev = lastConnection;
            lastConnection->next = link;
            lastConnection = link;
        }
        totalConnections++;
        actionHistory.push("ADD_FRIEND:" + targetUsername);
        cout << "  Connected with @" << targetUsername << ".\n";
    }

    void removeConnection(string targetUsername) {
        Connection* current = firstConnection;
        while (current != NULL) {
            if (isSameString(current->friendUsername, targetUsername)) {
                if (current->prev != NULL) current->prev->next = current->next;
                else                       firstConnection = current->next;
                if (current->next != NULL) current->next->prev = current->prev;
                else                       lastConnection = current->prev;

                totalConnections--;
                delete current;
                actionHistory.push("REMOVE_FRIEND:" + targetUsername);
                cout << "  Removed @" << targetUsername << " from connections.\n";
                return;
            }
            current = current->next;
        }
        cout << "  @" << targetUsername << " is not in your connections.\n";
    }

    void displayConnections() {
        if (firstConnection == NULL) {
            cout << "  No connections yet.\n";
            return;
        }
        Connection* current = firstConnection;
        while (current != NULL) {
            cout << "  - @" << current->friendUsername << "\n";
            current = current->next;
        }
    }

    void displayProfile() {
        cout << "\n================================\n";
        cout << "Profile: @" << username << "\n";
        cout << "================================\n";
        cout << "  Connections (" << totalConnections << "):\n";
        displayConnections();
        cout << "  Posts (" << totalPosts << "):\n";
        displayPosts();
    }

    ~User() {
        Post* post = firstPost;
        while (post != NULL) {
            Post* next = post->next;
            delete post;
            post = next;
        }
        Connection* conn = firstConnection;
        while (conn != NULL) {
            Connection* next = conn->next;
            delete conn;
            conn = next;
        }
    }
};


// ── UserTreeNode (BST node) ──────────────────────────────────────────────────

struct UserTreeNode {
    User* user;
    UserTreeNode* left;
    UserTreeNode* right;

    UserTreeNode(User* user) {
        this->user = user;
        this->left = NULL;
        this->right = NULL;
    }
};


// ── UserBST ──────────────────────────────────────────────────────────────────
// Binary Search Tree keyed on username (alphabetical order).
// Gives O(log n) average-case search and insert.

struct UserBST {
    UserTreeNode* root;

    UserBST() { root = NULL; }

    UserTreeNode* insertNode(UserTreeNode* node, User* user) {
        if (node == NULL) return new UserTreeNode(user);
        int direction = compareStrings(user->username, node->user->username);
        if (direction < 0) node->left = insertNode(node->left, user);
        else if (direction > 0) node->right = insertNode(node->right, user);
        return node;
    }

    void insert(User* user) {
        root = insertNode(root, user);
    }

    // Iterative search — no recursion needed, simpler to follow.
    User* findUser(string username) {
        UserTreeNode* current = root;
        while (current != NULL) {
            int direction = compareStrings(username, current->user->username);
            if (direction == 0) return current->user;
            else if (direction < 0) current = current->left;
            else                     current = current->right;
        }
        return NULL;
    }

    // Inorder traversal prints users in alphabetical order.
    void printInorder(UserTreeNode* node) {
        if (node == NULL) return;
        printInorder(node->left); 
        cout << "  @" << node->user->username
            << "  |  Connections: " << node->user->totalConnections
            << "  |  Posts: " << node->user->totalPosts << "\n";
        printInorder(node->right);
    }

    void displayAllUsers() {
        if (root == NULL) { cout << "  No users registered yet.\n"; return; }
        printInorder(root);
    }

    void destroyTree(UserTreeNode* node) {
        if (node == NULL) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node->user;
        delete node;
    }

    ~UserBST() { destroyTree(root); }
};


// ── File persistence ─────────────────────────────────────────────────────────
// Format per user (one block):
//   username password
//   connectionCount
//   connectionUsername  (one per line)
//   postCount
//   postId             (one per line)
//   postText           (one per line)

void saveAllData(UserBST& tree) {
    ofstream file("users.txt");
    if (!file.is_open()) { cout << "  Error: could not open users.txt for writing.\n"; return; }

    // Iterative inorder using a manual stack (array-based).
    UserTreeNode* nodeStack[500];
    int           stackTop = -1;
    UserTreeNode* current = tree.root;

    while (current != NULL || stackTop >= 0) {
        while (current != NULL) {
            nodeStack[++stackTop] = current;
            current = current->left;
        }
        current = nodeStack[stackTop--];
        User* u = current->user;

        file << u->username << " " << u->password << "\n";

        file << u->totalConnections << "\n";
        Connection* conn = u->firstConnection;
        while (conn != NULL) { file << conn->friendUsername << "\n"; conn = conn->next; }

        file << u->totalPosts << "\n";
        Post* post = u->firstPost;
        while (post != NULL) { file << post->id << "\n" << post->text << "\n"; post = post->next; }

        current = current->right;
    }

    file.close();
    cout << "  Data saved.\n";
}

void loadSavedData(UserBST& tree, GlobalFeed& feed, int& lastPostId) {
    ifstream file("users.txt");
    if (!file.is_open()) return; // No saved data yet — normal on first run.

    string username, password;
    while (file >> username >> password) {
        User* u = new User();
        u->username = username;
        u->password = password;

        int connectionCount;
        file >> connectionCount;
        for (int i = 0; i < connectionCount; i++) {
            string connName; file >> connName;
            Connection* link = new Connection(connName);
            if (u->firstConnection == NULL) {
                u->firstConnection = u->lastConnection = link;
            }
            else {
                link->prev = u->lastConnection;
                u->lastConnection->next = link;
                u->lastConnection = link;
            }
            u->totalConnections++;
        }

        int postCount;
        file >> postCount;
        file.ignore(); // consume the newline after postCount
        for (int i = 0; i < postCount; i++) {
            string idLine, textLine;
            getline(file, idLine);
            getline(file, textLine);
            int id = stringToInt(idLine);
            if (id > lastPostId) lastPostId = id;

            Post* post = new Post(id, textLine, username);
            if (u->firstPost == NULL) {
                u->firstPost = u->lastPost = post;
            }
            else {
                post->prev = u->lastPost;
                u->lastPost->next = post;
                u->lastPost = post;
            }
            u->totalPosts++;
            feed.addPost(post);
        }

        tree.insert(u);
    }

    file.close();
    cout << "  Session data restored.\n";
}


// ── Undo handler ─────────────────────────────────────────────────────────────
// Parses the top action token and reverses it.

void undoLastAction(User* user, UserBST& tree, GlobalFeed& feed) {
    if (user->actionHistory.isEmpty()) {
        cout << "  Nothing to undo.\n";
        return;
    }

    string token = user->actionHistory.pop();

    // Split token at ':' into action type and value.
    int colonAt = -1;
    for (int i = 0; i < (int)token.length(); i++) {
        if (token[i] == ':') { colonAt = i; break; }
    }
    string actionType = token.substr(0, colonAt);
    string actionValue = token.substr(colonAt + 1);

    if (isSameString(actionType, "ADD_FRIEND")) {
        // Remove the connection from both sides.
        Connection* current = user->firstConnection;
        while (current != NULL) {
            if (isSameString(current->friendUsername, actionValue)) {
                if (current->prev != NULL) current->prev->next = current->next;
                else                       user->firstConnection = current->next;
                if (current->next != NULL) current->next->prev = current->prev;
                else                       user->lastConnection = current->prev;
                user->totalConnections--;
                delete current;
                break;
            }
            current = current->next;
        }
        User* otherUser = tree.findUser(actionValue);
        if (otherUser != NULL) otherUser->removeConnection(user->username);
        cout << "  Undo: connection with @" << actionValue << " removed.\n";
    }
    else if (isSameString(actionType, "REMOVE_FRIEND")) {
        // Restore the connection on both sides.
        user->addConnection(actionValue);
        User* otherUser = tree.findUser(actionValue);
        if (otherUser != NULL) otherUser->addConnection(user->username);
        cout << "  Undo: reconnected with @" << actionValue << ".\n";
    }
    else if (isSameString(actionType, "CREATE_POST")) {
        int postId = stringToInt(actionValue);
        user->removePost(postId, feed);
        cout << "  Undo: post #" << postId << " removed.\n";
    }
    else {
        cout << "  Unrecognized action token.\n";
    }
}


// ── Menus ─────────────────────────────────────────────────────────────────────

void showDashboard(User* activeUser, UserBST& tree, GlobalFeed& feed, int& lastPostId) {
    int choice;
    do {

        cout << "\n========================================\n";
        cout << "Logged in as: @" << activeUser->username << "\n";
        cout << "========================================\n";
        cout << "1. View Profile\n";
        cout << "2. Create Post\n";
        cout << "3. Delete Post\n";
        cout << "4. Add Connection\n";
        cout << "5. Remove Connection\n";
        cout << "6. View Global Feed\n";
        cout << "7. Search User\n";
        cout << "8. Browse All Users\n";
        cout << "9. Undo Last Action\n";
        cout << "10. Save Data\n";
        cout << "0. Logout\n";
        cout << "========================================\n";
        cout << "  Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {

            activeUser->displayProfile();
        }
        else if (choice == 2) {
            string text;
            cout << "  Write your post: ";
            getline(cin, text);
            lastPostId++;
            activeUser->createPost(lastPostId, text, feed);
        }
        else if (choice == 3) {
            int id;
            cout << "  Enter post ID to delete: ";
            cin >> id;
            activeUser->removePost(id, feed);
        }
        else if (choice == 4) {
            string target;
            cout << "  Enter username to connect with: ";
            cin >> target;
            if (isSameString(target, activeUser->username)) {
                cout << "  You cannot connect with yourself.\n";
            }
            else {
                User* targetUser = tree.findUser(target);
                if (targetUser == NULL) {
                    cout << "  User @" << target << " does not exist.\n";
                }
                else {
                    activeUser->addConnection(target);
                    targetUser->addConnection(activeUser->username);
                }
            }
        }
        else if (choice == 5) {
            string target;
            cout << "  Enter username to disconnect: ";
            cin >> target;
            User* targetUser = tree.findUser(target);
            if (targetUser != NULL) targetUser->removeConnection(activeUser->username);
            activeUser->removeConnection(target);
        }
        else if (choice == 6) {
            cout << "\n========== GLOBAL FEED ==========\n";
            feed.display();
        }
        else if (choice == 7) {
            string target;
            cout << "  Enter username to search: ";
            cin >> target;
            User* result = tree.findUser(target);
            if (result == NULL) {
                cout << "  User @" << target << " not found.\n";
            }
            else {
                cout << "  Found: @" << result->username
                    << "  |  Connections: " << result->totalConnections
                    << "  |  Posts: " << result->totalPosts << "\n";
            }
        }
        else if (choice == 8) {
            cout << "\n========== ALL USERS (A-Z) ==========\n";
            tree.displayAllUsers();
        }
        else if (choice == 9) {
            undoLastAction(activeUser, tree, feed);
        }
        else if (choice == 10) {
            saveAllData(tree);
        }
        else if (choice == 0) {
            cout << "  Logged out.\n";
        }
        else {
            cout << "  Invalid option.\n";
        }


    } while (choice != 0);
}

void showMainMenu() {
    UserBST    userTree;
    GlobalFeed feed;
    int        lastPostId = 0;

    loadSavedData(userTree, feed, lastPostId);

    int choice;
    do {

        cout << "\n========================================\n";
        cout << "     SOCIAL MEDIA NETWORK SIMULATOR\n";
        cout << "        Data Structures Project\n";
        cout << "========================================\n";
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "0. Exit\n";
        cout << "========================================\n";
        cout << "  Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string username, password;
            cout << "  Username: ";
            cin >> username;
            if (userTree.findUser(username) != NULL) {
                cout << "  Username already taken.\n";
            }
            else {
                cout << "  Password: ";
                cin >> password;
                User* newUser = new User();
                newUser->username = username;
                newUser->password = password;
                userTree.insert(newUser);
                cout << "  Account created. Welcome, @" << username << "!\n";
            }
        }
        else if (choice == 2) {
            string username, password;
            cout << "  Username: ";
            cin >> username;
            cout << "  Password: ";
            cin >> password;
            User* user = userTree.findUser(username);
            if (user == NULL || !isSameString(user->password, password)) {
                cout << "  Incorrect username or password.\n";
            }
            else {
                cout << "  Login successful.\n";
                showDashboard(user, userTree, feed, lastPostId);
            }
        }
        else if (choice == 0) {
            saveAllData(userTree);
            cout << "  Goodbye.\n";
        }
        else {
            cout << "  Invalid choice.\n";
        }

    } while (choice != 0);
}


// ── Entry point ───────────────────────────────────────────────────────────────



int main() {
    showMainMenu();
    return 0;
}