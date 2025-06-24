#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>

const int BOARD_SIZE = 3;
const int MAX_TOKENS = BOARD_SIZE;
const int CELL_SIZE = 100;  // Size of each cell in pixels

// Structure to hold game state
struct GameState {
    int playerA_tokens[MAX_TOKENS][2];  // [i][0] = row, [i][1] = col
    int playerB_tokens[MAX_TOKENS][2];  // [i][0] = row, [i][1] = col
    char currentPlayer;
};

// Simple stack implementation
const int MAX_STACK_SIZE = 100;
GameState stateStack[MAX_STACK_SIZE];
int stackTop = -1;

void pushState(const GameState& state) {
    if (stackTop < MAX_STACK_SIZE - 1) {
        stateStack[++stackTop] = state;
    }
}

GameState popState() {
    if (stackTop >= 0) {
        return stateStack[stackTop--];
    }
    return stateStack[0]; // Return empty state if stack is empty
}

bool isStackEmpty() {
    return stackTop == -1;
}

// Global current state
GameState currentState;

// Function declarations
void initializeGame();
bool hasWon(char player);
bool isPositionEmpty(int row, int col);
bool isOnBoard(int row, int col);
void getAllPossibleMoves(int moves[][4], int* moveCount);
void applyMove(int move[4]);
char getOpponent(char player);
char evaluateGameState();
bool findBestMove(int bestMove[4]);
int findTokenAtPosition(int row, int col);
bool getValidMoveFromPosition(int row, int col, int move[4]);
bool hasValidMoves(char player);  // NEW FUNCTION to check if a player has valid moves

// Draw the game board using SFML
void drawBoard(sf::RenderWindow& window, sf::Sprite playerA_sprites[], sf::Sprite playerB_sprites[],
    sf::Font& font, const std::string& message, sf::RectangleShape& restartButton, sf::Text& restartText) {
    // Clear previous frame
    window.clear(sf::Color::Black);

    // Draw the grid
    for (int row = 0; row <= BOARD_SIZE + 1; row++) {
        for (int col = 0; col <= BOARD_SIZE + 1; col++) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
            cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);

            // Color the cells based on their position
            if (row == 0 || row == BOARD_SIZE + 1 || col == 0 || col == BOARD_SIZE + 1) {
                // Border cells
                if ((row == 0 || row == BOARD_SIZE + 1) && col >= 1 && col <= BOARD_SIZE) {
                    // Top and bottom borders (Player B goal)
                    cell.setFillColor(sf::Color(100, 255, 100)); // Green for player B
                }
                else if ((col == 0 || col == BOARD_SIZE + 1) && row >= 1 && row <= BOARD_SIZE) {
                    // Left and right borders (Player A goal)
                    cell.setFillColor(sf::Color(255, 100, 100)); // Red for player A
                }
                else {
                    // Corner cells
                    cell.setFillColor(sf::Color(150, 150, 150));
                }
            }
            else {
                // Interior cells
                cell.setFillColor(sf::Color(200, 200, 200));
            }

            window.draw(cell);
        }
    }

    // Draw Player A tokens
    for (int i = 0; i < MAX_TOKENS; i++) {
        int row = currentState.playerA_tokens[i][0];
        int col = currentState.playerA_tokens[i][1];

        if (isOnBoard(row, col)) {
            // Position at the center of the cell
            playerA_sprites[i].setPosition(col * CELL_SIZE + CELL_SIZE / 2, row * CELL_SIZE + CELL_SIZE / 2);
            window.draw(playerA_sprites[i]);
        }
    }

    // Draw Player B tokens
    for (int i = 0; i < MAX_TOKENS; i++) {
        int row = currentState.playerB_tokens[i][0];
        int col = currentState.playerB_tokens[i][1];

        if (isOnBoard(row, col)) {
            // Position at the center of the cell
            playerB_sprites[i].setPosition(col * CELL_SIZE + CELL_SIZE / 2, row * CELL_SIZE + CELL_SIZE / 2);
            window.draw(playerB_sprites[i]);
        }
    }

    // Display current player turn indicator
    sf::Text turnText;
    turnText.setFont(font);
    turnText.setCharacterSize(20);
    turnText.setPosition(10, (BOARD_SIZE + 2) * CELL_SIZE + 10);

    if (currentState.currentPlayer == 'A') {
        turnText.setString("Player A's Turn (You)");
        turnText.setFillColor(sf::Color::Red);
    }
    else {
        turnText.setString("Player B's Turn (Computer)");
        turnText.setFillColor(sf::Color::Green);
    }

    window.draw(turnText);

    // Display message
    sf::Text messageText;
    messageText.setFont(font);
    messageText.setCharacterSize(16);
    messageText.setFillColor(sf::Color::White);
    messageText.setPosition(10, (BOARD_SIZE + 2) * CELL_SIZE + 40);
    messageText.setString(message);
    window.draw(messageText);

    // Draw restart button
    window.draw(restartButton);
    window.draw(restartText);
}

int main() {
    initializeGame();

    // Create window with additional height for the restart button
    sf::RenderWindow window(sf::VideoMode((BOARD_SIZE + 2) * CELL_SIZE, (BOARD_SIZE + 2) * CELL_SIZE + 100),
        "Token Movement Game", sf::Style::Close);

    // Load font
    sf::Font font;
    bool fontLoaded = font.loadFromFile("arial.ttf");
    if (!fontLoaded) {
        std::cerr << "Warning: Could not load font." << std::endl;
        // Try system font (Windows)
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            std::cerr << "Warning: Could not load system font." << std::endl;
        }
    }

    // Load custom token textures for both players
    sf::Texture textureA, textureB;

    // Try to load shield token textures
    bool texturesLoaded = textureA.loadFromFile("C:\\Users\\LENOVO\\Desktop\\redSprite.png.png") &&
        textureB.loadFromFile("C:\\Users\\LENOVO\\Desktop\\greenSprite.png.png");

    // If textures can't be loaded, use colored circles instead
    if (!texturesLoaded) {
        std::cout << "Could not load shield textures, using colored circles instead.\n";

        // Create images for the fallback textures
        sf::Image imgA, imgB;
        imgA.create(CELL_SIZE, CELL_SIZE, sf::Color::Transparent);
        imgB.create(CELL_SIZE, CELL_SIZE, sf::Color::Transparent);

        // Draw circles on the images
        for (unsigned int x = 0; x < CELL_SIZE; x++) {
            for (unsigned int y = 0; y < CELL_SIZE; y++) {
                float dx = x - CELL_SIZE / 2.0f;
                float dy = y - CELL_SIZE / 2.0f;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Make the circle radius smaller
                if (distance < CELL_SIZE / 3.5f) {  // Smaller radius (was CELL_SIZE / 2.5f)
                    imgA.setPixel(x, y, sf::Color::Red);
                    imgB.setPixel(x, y, sf::Color::Green);
                }
            }
        }

        textureA.update(imgA);
        textureB.update(imgB);
    }

    // Create sprites for each player
    sf::Sprite playerA_sprites[MAX_TOKENS];
    sf::Sprite playerB_sprites[MAX_TOKENS];

    for (int i = 0; i < MAX_TOKENS; i++) {
        playerA_sprites[i].setTexture(textureA);
        playerB_sprites[i].setTexture(textureB);

        // Set the origin to be the center of the sprite
        playerA_sprites[i].setOrigin(textureA.getSize().x / 2, textureA.getSize().y / 2);
        playerB_sprites[i].setOrigin(textureB.getSize().x / 2, textureB.getSize().y / 2);

        // Scale sprites to be smaller than the grid cell
        if (texturesLoaded) {
            float scaleA = static_cast<float>(CELL_SIZE * 0.7) / textureA.getSize().x;  // 70% of cell size
            float scaleB = static_cast<float>(CELL_SIZE * 0.7) / textureB.getSize().x;  // 70% of cell size
            playerA_sprites[i].setScale(scaleA, scaleA);
            playerB_sprites[i].setScale(scaleB, scaleB);
        }
    }

    // Create restart button
    sf::RectangleShape restartButton(sf::Vector2f(120, 30));
    restartButton.setFillColor(sf::Color(128, 128, 128));  // grey button
    restartButton.setPosition((BOARD_SIZE + 2) * CELL_SIZE - 130, (BOARD_SIZE + 2) * CELL_SIZE + 65);

    // Create restart button text
    sf::Text restartText;
    restartText.setFont(font);
    restartText.setString("Restart");
    restartText.setCharacterSize(18);
    restartText.setFillColor(sf::Color::White);

    // Center the text on the button
    sf::FloatRect textBounds = restartText.getLocalBounds();
    restartText.setPosition(
        restartButton.getPosition().x + (restartButton.getSize().x - textBounds.width) / 2,
        restartButton.getPosition().y + (restartButton.getSize().y - textBounds.height) / 2 - 5
    );

    std::string message = "Click on a token to move it. Player A moves right, Player B moves down.";
    bool waitForComputerMove = false;
    sf::Clock clock;
    bool gameOver = false;

    std::cout << "Welcome to the Token Movement Game!\n";
    std::cout << "Player A (You) moves right, Player B (Computer) moves down.\n";
    std::cout << "First to move all tokens off the board wins!\n\n";

    // Main game loop
    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // Check if restart button was clicked
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                if (restartButton.getGlobalBounds().contains(mousePos)) {
                    std::cout << "Restarting game!\n";
                    initializeGame();  // Reset game state
                    message = "Game restarted! Click on a token to move it.";
                    waitForComputerMove = false;
                    gameOver = false;
                }
                // Process game moves only if not game over and it's player's turn
                else if (!gameOver && currentState.currentPlayer == 'A') {
                    // Check if Player A has valid moves
                    if (!hasValidMoves('A')) {
                        std::cout << "Player A has no valid moves. Turn passes to Player B.\n";
                        message = "Player A has no valid moves. Turn passes to Player B.";
                        currentState.currentPlayer = 'B';
                        waitForComputerMove = true;
                        clock.restart();
                    }
                    else {
                        // Convert mouse position to grid coordinates
                        int clickCol = event.mouseButton.x / CELL_SIZE;
                        int clickRow = event.mouseButton.y / CELL_SIZE;

                        std::cout << "Clicked at position: (" << clickRow << "," << clickCol << ")\n";

                        // Check if a valid token was clicked
                        int move[4];
                        if (getValidMoveFromPosition(clickRow, clickCol, move)) {
                            std::cout << "Valid move from (" << move[0] << "," << move[1]
                                << ") to (" << move[2] << "," << move[3] << ")\n";

                            message = "Moving from (" + std::to_string(move[0]) + "," + std::to_string(move[1]) +
                                ") to (" + std::to_string(move[2]) + "," + std::to_string(move[3]) + ")";

                            applyMove(move);
                            currentState.currentPlayer = 'B';  // Switch to computer
                            waitForComputerMove = true;
                            clock.restart();  // Reset the clock for computer's turn delay
                        }
                        else {
                            std::cout << "Invalid selection or no valid moves from this position.\n";
                            message = "Invalid selection or no valid moves from position (" +
                                std::to_string(clickRow) + "," + std::to_string(clickCol) + ").";
                        }
                    }
                }
            }
        }

        // Computer's turn with delay
        if (waitForComputerMove && clock.getElapsedTime().asSeconds() > 1.0f) {  // 1 second delay
            // Check if Computer has valid moves
            if (!hasValidMoves('B')) {
                std::cout << "Computer (Player B) has no valid moves. Turn passes to Player A.\n";
                message = "Computer has no valid moves. Turn passes to Player A.";
                currentState.currentPlayer = 'A';
            }
            else {
                int computerMove[4];
                if (findBestMove(computerMove)) {
                    std::cout << "Computer moves from (" << computerMove[0] << "," << computerMove[1]
                        << ") to (" << computerMove[2] << "," << computerMove[3] << ")\n";

                    message = "Computer moved from (" + std::to_string(computerMove[0]) + "," + std::to_string(computerMove[1]) +
                        ") to (" + std::to_string(computerMove[2]) + "," + std::to_string(computerMove[3]) + ")";

                    applyMove(computerMove);
                }
                else {
                    std::cout << "Computer has no valid moves. Turn passes.\n";
                    message = "Computer has no valid moves. Turn passes.";
                }
                currentState.currentPlayer = 'A';  // Switch back to player
            }
            waitForComputerMove = false;
        }

        // Check for win conditions
        if (hasWon('A')) {
            message = "Player A wins! Click Restart to play again.";
            gameOver = true;
        }
        else if (hasWon('B')) {
            message = "Player B (Computer) wins! Click Restart to play again.";
            gameOver = true;
        }

        // Draw game state
        drawBoard(window, playerA_sprites, playerB_sprites, font, message, restartButton, restartText);

        // Display everything
        window.display();

        // Add a small delay to prevent CPU hogging
        sf::sleep(sf::milliseconds(50));
    }

    return 0;
}

// NEW FUNCTION to check if a player has valid moves
bool hasValidMoves(char player) {
    // Temporarily set the current player
    char savedPlayer = currentState.currentPlayer;
    currentState.currentPlayer = player;

    // Get all possible moves
    int moves[MAX_TOKENS * 2][4];
    int moveCount = 0;
    getAllPossibleMoves(moves, &moveCount);

    // Restore the original player
    currentState.currentPlayer = savedPlayer;

    // Return true if there is at least one valid move
    return moveCount > 0;
}

void initializeGame() {
    currentState.currentPlayer = 'A';

    // Initialize Player A tokens (left border)
    for (int i = 0; i < MAX_TOKENS; i++) {
        currentState.playerA_tokens[i][0] = i + 1;  // row
        currentState.playerA_tokens[i][1] = 0;      // col
    }

    // Initialize Player B tokens (top border)
    for (int j = 0; j < MAX_TOKENS; j++) {
        currentState.playerB_tokens[j][0] = 0;      // row
        currentState.playerB_tokens[j][1] = j + 1;  // col
    }

    // Reset stack
    stackTop = -1;
}

bool hasWon(char player) {
    if (player == 'A') {
        // Check if all Player A tokens are off the right edge
        for (int i = 0; i < MAX_TOKENS; i++) {
            if (currentState.playerA_tokens[i][1] <= BOARD_SIZE) return false;
        }
        return true;
    }
    else {
        // Check if all Player B tokens are off the bottom edge
        for (int j = 0; j < MAX_TOKENS; j++) {
            if (currentState.playerB_tokens[j][0] <= BOARD_SIZE) return false;
        }
        return true;
    }
}

bool isPositionEmpty(int row, int col) {
    // Check if position is occupied by any token
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (currentState.playerA_tokens[i][0] == row && currentState.playerA_tokens[i][1] == col) return false;
        if (currentState.playerB_tokens[i][0] == row && currentState.playerB_tokens[i][1] == col) return false;
    }
    return true;
}

bool isOnBoard(int row, int col) {
    return row >= 0 && row <= BOARD_SIZE + 1 &&
        col >= 0 && col <= BOARD_SIZE + 1;
}

int findTokenAtPosition(int row, int col) {
    // Check for player A tokens
    if (currentState.currentPlayer == 'A') {
        for (int i = 0; i < MAX_TOKENS; i++) {
            if (currentState.playerA_tokens[i][0] == row && currentState.playerA_tokens[i][1] == col) {
                return i;
            }
        }
    }
    // Check for player B tokens
    else {
        for (int i = 0; i < MAX_TOKENS; i++) {
            if (currentState.playerB_tokens[i][0] == row && currentState.playerB_tokens[i][1] == col) {
                return i;
            }
        }
    }
    return -1; // No token found
}

// Function to get a valid move from a specific position
bool getValidMoveFromPosition(int row, int col, int move[4]) {
    // Find if there's a token at the selected position
    int tokenIndex = findTokenAtPosition(row, col);

    if (tokenIndex == -1) {
        return false; // No token at the selected position
    }

    // Set the from position in array
    move[0] = row;
    move[1] = col;

    if (currentState.currentPlayer == 'A') {
        // Player A moves right (horizontal)

        // Try 1-step move to the right
        int toRow = row;
        int toCol = col + 1;

        if (isOnBoard(toRow, toCol) && isPositionEmpty(toRow, toCol)) {
            move[2] = toRow;
            move[3] = toCol;
            return true;
        }

        // Try 2-step jump over opponent
        toCol = col + 2;
        if (isOnBoard(toRow, toCol) && isPositionEmpty(toRow, toCol)) {
            // Check if there's an opponent in between
            bool opponentBetween = false;
            for (int j = 0; j < MAX_TOKENS; j++) {
                if (currentState.playerB_tokens[j][0] == row && currentState.playerB_tokens[j][1] == col + 1) {
                    opponentBetween = true;
                    break;
                }
            }

            if (opponentBetween) {
                move[2] = toRow;
                move[3] = toCol;
                return true;
            }
        }
    }
    else {
        // Player B moves down (vertical)

        // Try 1-step move down
        int toRow = row + 1;
        int toCol = col;

        if (isOnBoard(toRow, toCol) && isPositionEmpty(toRow, toCol)) {
            move[2] = toRow;
            move[3] = toCol;
            return true;
        }

        // Try 2-step jump over opponent
        toRow = row + 2;
        if (isOnBoard(toRow, toCol) && isPositionEmpty(toRow, toCol)) {
            // Check if there's an opponent in between
            bool opponentBetween = false;
            for (int i = 0; i < MAX_TOKENS; i++) {
                if (currentState.playerA_tokens[i][0] == row + 1 && currentState.playerA_tokens[i][1] == col) {
                    opponentBetween = true;
                    break;
                }
            }

            if (opponentBetween) {
                move[2] = toRow;
                move[3] = toCol;
                return true;
            }
        }
    }

    return false; // No valid move from this position
}

void getAllPossibleMoves(int moves[][4], int* moveCount) {
    *moveCount = 0;

    if (currentState.currentPlayer == 'A') {
        // Player A moves right (horizontal)
        for (int i = 0; i < MAX_TOKENS; i++) {
            int fromRow = currentState.playerA_tokens[i][0];
            int fromCol = currentState.playerA_tokens[i][1];

            // Check 1-step move to the right
            int toRow = fromRow;
            int toCol = fromCol + 1;
            if (isOnBoard(toRow, toCol)) {
                if (isPositionEmpty(toRow, toCol)) {
                    moves[*moveCount][0] = fromRow;
                    moves[*moveCount][1] = fromCol;
                    moves[*moveCount][2] = toRow;
                    moves[*moveCount][3] = toCol;
                    (*moveCount)++;
                }
                else {
                    // Check jump over opponent (2 steps)
                    toCol = fromCol + 2;
                    if (isOnBoard(toRow, toCol)) {
                        bool opponentBetween = false;
                        for (int j = 0; j < MAX_TOKENS; j++) {
                            if (currentState.playerB_tokens[j][0] == fromRow && currentState.playerB_tokens[j][1] == fromCol + 1) {
                                opponentBetween = true;
                                break;
                            }
                        }
                        if (opponentBetween && isPositionEmpty(toRow, toCol)) {
                            moves[*moveCount][0] = fromRow;
                            moves[*moveCount][1] = fromCol;
                            moves[*moveCount][2] = toRow;
                            moves[*moveCount][3] = toCol;
                            (*moveCount)++;
                        }
                    }
                }
            }
        }
    }
    else {
        // Player B moves down (vertical)
        for (int j = 0; j < MAX_TOKENS; j++) {
            int fromRow = currentState.playerB_tokens[j][0];
            int fromCol = currentState.playerB_tokens[j][1];

            // Check 1-step move down
            int toRow = fromRow + 1;
            int toCol = fromCol;
            if (isOnBoard(toRow, toCol)) {
                if (isPositionEmpty(toRow, toCol)) {
                    moves[*moveCount][0] = fromRow;
                    moves[*moveCount][1] = fromCol;
                    moves[*moveCount][2] = toRow;
                    moves[*moveCount][3] = toCol;
                    (*moveCount)++;
                }
                else {
                    // Check jump over opponent (2 steps)
                    toRow = fromRow + 2;
                    if (isOnBoard(toRow, toCol)) {
                        bool opponentBetween = false;
                        for (int i = 0; i < MAX_TOKENS; i++) {
                            if (currentState.playerA_tokens[i][0] == fromRow + 1 && currentState.playerA_tokens[i][1] == fromCol) {
                                opponentBetween = true;
                                break;
                            }
                        }
                        if (opponentBetween && isPositionEmpty(toRow, toCol)) {
                            moves[*moveCount][0] = fromRow;
                            moves[*moveCount][1] = fromCol;
                            moves[*moveCount][2] = toRow;
                            moves[*moveCount][3] = toCol;
                            (*moveCount)++;
                        }
                    }
                }
            }
        }
    }
}

void applyMove(int move[4]) {
    if (currentState.currentPlayer == 'A') {
        // Update Player A token position
        for (int i = 0; i < MAX_TOKENS; i++) {
            if (currentState.playerA_tokens[i][0] == move[0] && currentState.playerA_tokens[i][1] == move[1]) {
                currentState.playerA_tokens[i][0] = move[2];
                currentState.playerA_tokens[i][1] = move[3];
                break;
            }
        }
    }
    else {
        // Update Player B token position
        for (int j = 0; j < MAX_TOKENS; j++) {
            if (currentState.playerB_tokens[j][0] == move[0] && currentState.playerB_tokens[j][1] == move[1]) {
                currentState.playerB_tokens[j][0] = move[2];
                currentState.playerB_tokens[j][1] = move[3];
                break;
            }
        }
    }
}

char getOpponent(char player) {
    return (player == 'A') ? 'B' : 'A';
}

char evaluateGameState() {
    // Base cases
    if (hasWon(currentState.currentPlayer)) return 'g';  // good
    if (hasWon(getOpponent(currentState.currentPlayer))) return 'b';  // bad

    // Save current state
    GameState savedState = currentState;
    pushState(savedState);

    int moves[MAX_TOKENS * 2][4];  // Worst case: each token has 2 possible moves
    int moveCount = 0;
    getAllPossibleMoves(moves, &moveCount);

    // Limit recursion depth for efficiency
    static int depth = 0;
    depth++;

    // Stop at depth 3 to prevent stack overflow
    if (depth > 3) {
        depth--;
        currentState = popState();
        return 'n'; // neutral
    }

    // Check all possible moves
    for (int i = 0; i < moveCount; i++) {
        applyMove(moves[i]);
        currentState.currentPlayer = getOpponent(currentState.currentPlayer);
        char result = evaluateGameState();
        if (result == 'b') {
            // Restore state
            currentState = popState();
            depth--;
            return 'g';  // Found a winning move
        }

        // Restore state for next iteration
        currentState = popState();
        pushState(savedState);
    }

    // Restore state
    currentState = popState();
    depth--;
    return 'b';  // All moves lead to opponent winning
}

bool findBestMove(int bestMove[4]) {
    int moves[MAX_TOKENS * 2][4];  // Worst case: each token has 2 possible moves
    int moveCount = 0;
    getAllPossibleMoves(moves, &moveCount);

    if (moveCount == 0) return false;

    // Save current state
    GameState savedState = currentState;
    pushState(savedState);

    // Try to find a move that leads to opponent's bad state
    for (int i = 0; i < moveCount; i++) {
        applyMove(moves[i]);
        currentState.currentPlayer = getOpponent(currentState.currentPlayer);

        if (evaluateGameState() == 'b') {
            // Found a winning move
            for (int j = 0; j < 4; j++) {
                bestMove[j] = moves[i][j];
            }
            // Restore state
            currentState = popState();
            return true;
        }

        // Restore state for next iteration
        currentState = popState();
        pushState(savedState);
    }

    // If no guaranteed win, return first valid move
    for (int j = 0; j < 4; j++) {
        bestMove[j] = moves[0][j];
    }
    currentState = popState();
    return true;
}