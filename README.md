# Social Media Network Simulator
### Data Structures Semester Project — University of Central Punjab

A fully functional terminal-based social media platform built in C++ where every feature is powered by a core data structure. No standard algorithm libraries were used — everything from string comparison to tree traversal was implemented from scratch.

---

## Team Members

- **Muhammad Asmar Naeem**
- **Mahatb Noor**
- **Ehtesham Arshad**

---

## Data Structures Used

| Data Structure | Where Used | Purpose |
|---|---|---|
| Binary Search Tree | User storage | O(log n) search and insert by username |
| Doubly Linked List | Posts and connections | Traversal in both directions, easy deletion |
| Stack (LIFO) | Undo history | Reverse the most recent action first |
| Queue (FIFO) | Global feed | Display posts in chronological order |
| File I/O | Data persistence | Save and restore all data between sessions |

---

## Features

- Register and login with username and password
- Create and delete posts
- Add and remove friend connections (two sided)
- View your personal profile with all posts and connections
- View the global feed showing all posts from all users in order
- Search any registered user by username
- Browse all registered users in alphabetical order
- Undo your last action (add friend, remove friend, or create post)
- Save data manually at any time
- All data automatically saved on exit and restored on next launch

---


## Project Structure

```
social_media/
    main.cpp        — complete source code
    README.md       — project documentation
```

---

## How It Works

### User Storage — Binary Search Tree
All registered users are stored in a BST keyed on username. This gives O(log n) average case performance for both inserting new users and searching for existing ones. Users are stored and displayed in alphabetical order using inorder traversal.

### Posts and Connections — Doubly Linked List
Every user owns two doubly linked lists — one for their posts and one for their friend connections. The doubly linked structure allows efficient insertion at the end and deletion from any position without traversing from the start.

### Undo System — Stack
Every reversible action is recorded as a token string and pushed onto the user's personal undo stack. Tokens follow the format ACTION:value for example ADD_FRIEND:sara or CREATE_POST:5. When undo is triggered the top token is popped, parsed, and the exact opposite action is performed.

### Global Feed — Queue
Every post created by any user is added to a single global FIFO queue. Posts are displayed in the exact order they were created. A separate FeedEntry wrapper node is used so the queue and the user's personal post list remain independent of each other.

### File Persistence
On exit all user data is written to users.txt in a structured format including usernames, passwords, connections, and posts. On the next launch this file is read back and every user object, linked list, and post is fully reconstructed in memory exactly as it was left.

---

## Implementation Highlights

- All string utility functions written from scratch including comparison, integer to string, and string to integer conversion
- Iterative inorder BST traversal using a manual array based stack to avoid recursion overflow on large trees
- Two sided connection management ensuring both users always stay in sync
- Undo system covers ADD_FRIEND, REMOVE_FRIEND, and CREATE_POST actions
- File parser uses getline for post text to correctly handle spaces in content

---

## Sample Terminal Output

```
========================================
     SOCIAL MEDIA NETWORK SIMULATOR
        Data Structures Project
========================================
1. Register
2. Login
0. Exit
========================================
  Choice: 2
  Username: asmar
  Password: 1234
  Login successful.

========================================
Logged in as: @asmar
========================================
1. View Profile
2. Create Post
3. Delete Post
4. Add Connection
5. Remove Connection
6. View Global Feed
7. Search User
8. Browse All Users
9. Undo Last Action
10. Save Data
0. Logout
========================================
```

---

## Course Information

**Course:** Data Structures and Algorithms
**Institution:** University of Central Punjab, Lahore
**Semester:** 4th Semester

---

## License

This project was created for academic purposes as a semester project submission.
