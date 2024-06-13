namespace Game {
    class Player {

        string GetName() {
            return "Name";
        }
    }

    void Test() {
        Player p;
    }
}

void Test() {
    Game::Player p;
}