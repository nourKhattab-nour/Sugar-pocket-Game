# Sugar-pocket-Game
🚀 Features
5×5 Grid Board: 3 tokens per player (Red moves right, Green moves down).

AI Opponent: Uses recursion + backtracking to simulate decision-making.

Game Rules:

Move: 1 step to adjacent empty square.

Jump: 2 steps over an opponent’s token.

Win Condition: Move all tokens to the opposite edge.

Time Complexity: O(bᵈ) (Branching factor *b=6*, depth *d=3*).
----------------------------------------------------------------------------------------------------------------------------------------------------

⚙️ Tech Stack
Languages: Java (GUI), C++ (Logic)

Algorithms: Backtracking, Game Tree Traversal

Tools: NetBeans IDE, SFML (Graphics)

--------------------------------------------------------------------------------------------------------------------------------------------------

🧠 AI Logic
Game Tree: Each node represents a board state; edges are legal moves.

Recursive Backtracking:

Explores all possible moves up to depth 3.

Prunes paths where opponent wins (return 'b').

Terminal States:

Win: All tokens reach opposite edge.

Draw: No legal moves left.

--------------------------------------------------------------------------------------------------------------------------------------------------

📜 Credits
Team: Nour Khattab, Hala Mohamed, Arwa Hamdi

Supervisors: Dr. Mostafa Salama, TA Lobna Hisham

Institution: British University in Egypt
