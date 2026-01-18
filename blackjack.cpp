#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <random>

using namespace std;

/**
 * Card klasa - Modeluje entitet karte.
 * Koristimo enum za tipove i vrednosti radi tipski bezbednog programiranja (Type Safety).
 */
class Card {
public:
    enum Suit { HEARTS, DIAMONDS, CLUBS, SPADES };
    enum Rank { ACE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING };

    Card(Rank r, Suit s) : rank(r), suit(s) {}

    // Metoda za dobijanje Blackjack vrednosti karte
    int getValue() const {
        if (rank > 10) return 10; // Slike se uvek racunaju kao 10
        if (rank == ACE) return 11; // As je po defaultu 11, logika za korekciju je u calculatePoints
        return (int)rank;
    }

    Rank getRank() const { return rank; }
    Suit getSuit() const { return suit; }

private:
    Rank rank;
    Suit suit;
};

/**
 * Deck klasa - Upravlja kolekcijom objekata klase Card.
 * Implementira inicijalizaciju i shuffle logiku.
 */
class Deck {
private:
    vector<Card> cards;
public:
    Deck() {
        // Konstruktor popunjava spil standardnim setom od 52 karte
        for (int s = Card::HEARTS; s <= Card::SPADES; ++s) {
            for (int r = Card::ACE; r <= Card::KING; ++r) {
                cards.push_back(Card((Card::Rank)r, (Card::Suit)s));
            }
        }
    }

    // Koristi se random_device za kvalitetnije generisanje nasumicnog redosleda
    void shuffleDeck() {
        auto rd = random_device {}; 
        auto rng = default_random_engine { rd() };
        shuffle(cards.begin(), cards.end(), rng);
    }

    Card drawCard() {
        Card drawn = cards.back();
        cards.pop_back();
        return drawn;
    }
};

/**
 * Racunanje ukupnog zbira ruku sa "Soft Hand" logikom za Asa.
 */
int calculatePoints(const vector<Card>& hand) {
    int total = 0;
    int aceCount = 0;
    
    for (const auto& card : hand) {
        int val = card.getValue();
        total += val;
        if (val == 11) aceCount++;
    }
    
    // Ako zbir prelazi 21, konvertujemo Ase iz 11 u 1 dok ne budemo "safe"
    while (total > 21 && aceCount > 0) {
        total -= 10;
        aceCount--;
    }
    return total;
}

/**
 * drawHand - Vizuelni prikaz stanja u memoriji (vector) u terminalu.
 * Formatirano sa fiksnim spacingom radi ocuvanja ivica ASCII grafike.
 */
void drawHand(const vector<Card>& hand, string owner, bool hideFirst = false) {
    string ranks[] = {"", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    string suitChars[] = {"H", "D", "C", "S"};
    string suitFull[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
    
    cout << "\n--- " << owner << " ---" << endl;
    
    // Renderovanje gornje ivice
    for (size_t i = 0; i < hand.size(); i++) cout << "┌─────────┐ ";
    cout << endl;

    // Renderovanje ranga - Uslovna logika za "10" jer zauzima 2 karaktera
    for (size_t i = 0; i < hand.size(); i++) {
        if (i == 0 && hideFirst) cout << "│ ?       │ ";
        else {
            string r = ranks[hand[i].getRank()];
            if (r == "10") cout << "│ " << r << "      │ ";
            else cout << "│ " << r << "       │ ";
        }
    }
    cout << endl;

    // Renderovanje simbola (Suit)
    for (size_t i = 0; i < hand.size(); i++) {
        if (i == 0 && hideFirst) cout << "│    ?    │ ";
        else cout << "│    " << suitChars[hand[i].getSuit()] << "    │ ";
    }
    cout << endl;

    // Renderovanje donjeg ranga (mirroring)
    for (size_t i = 0; i < hand.size(); i++) {
        if (i == 0 && hideFirst) cout << "│       ? │ ";
        else {
            string r = ranks[hand[i].getRank()];
            if (r == "10") cout << "│      " << r << " │ ";
            else cout << "│       " << r << " │ ";
        }
    }
    cout << endl;

    for (size_t i = 0; i < hand.size(); i++) cout << "└─────────┘ ";
    cout << endl;

    // Tekstualni log radi lakseg debugovanja/pracenja
    cout << "Log: ";
    for (size_t i = 0; i < hand.size(); i++) {
        if (i == 0 && hideFirst) cout << "[HIDDEN] ";
        else cout << "[" << ranks[hand[i].getRank()] << " " << suitFull[hand[i].getSuit()] << "] ";
    }
    cout << endl;
}

int main() {
    // Seed za nasumicne brojeve baziran na sistemskom vremenu
    srand(time(0));
    int balance = 1000;
    char playAgain = 'y';

    cout << "===================================" << endl;
    cout << "        BLACKJACK ENGINE           " << endl;
    cout << "===================================" << endl;

    // Glavni Game Loop
    while (balance > 0 && (playAgain == 'y' || playAgain == 'Y')) {
        int bet;
        cout << "\nBALANCE: $" << balance << " | Enter bet: ";
        cin >> bet;

        // Validacija unosa uloga (Input validation)
        if (bet > balance || bet <= 0) {
            cout << "Invalid bet! Input must be between 1 and current balance." << endl;
            continue;
        }

        Deck deck;
        deck.shuffleDeck();

        // Inicijalno deljenje karata
        vector<Card> playerHand = {deck.drawCard(), deck.drawCard()};
        vector<Card> dealerHand = {deck.drawCard(), deck.drawCard()};

        // --- IGRACEV CIKLUS ---
        bool playerActive = true;
        while (playerActive) {
            drawHand(dealerHand, "DEALER HAND", true);
            drawHand(playerHand, "PLAYER HAND", false);
            
            int p = calculatePoints(playerHand);
            cout << "\nYOUR TOTAL: " << p << endl;

            if (p >= 21) break;

            cout << "ACTION: [h] Hit | [s] Stand: ";
            char choice; cin >> choice;
            
            if (choice == 'h') playerHand.push_back(deck.drawCard());
            else playerActive = false;
        }

        // --- EVALUACIJA I RED DILERA ---
        int playerFinal = calculatePoints(playerHand);
        
        if (playerFinal > 21) {
            cout << "\nBUST! Player went over 21. Balance -$" << bet << endl;
            balance -= bet;
        } else {
            cout << "\n--- DEALER'S TURN ---" << endl;
            // Dealer AI: Mora da vuce dok ne dostigne barem 17 (Soft 17 rule)
            while (calculatePoints(dealerHand) < 17) dealerHand.push_back(deck.drawCard());
            
            drawHand(dealerHand, "FINAL DEALER HAND");
            int dealerFinal = calculatePoints(dealerHand);

            if (dealerFinal > 21 || playerFinal > dealerFinal) {
                cout << "WINNER! Player beats Dealer. Balance +$" << bet << endl;
                balance += bet;
            } else if (dealerFinal > playerFinal) {
                cout << "DEALER WINS! Player loses. Balance -$" << bet << endl;
                balance -= bet;
            } else {
                cout << "PUSH! It's a tie. Bet returned." << endl;
            }
        }

        if (balance > 0) {
            cout << "\nPlay another round? (y/n): "; cin >> playAgain;
        } else {
            cout << "\nGAME OVER! Zero balance." << endl;
        }
    }

    cout << "\nFINAL BALANCE: $" << balance << "\nTerminating session..." << endl;
    return 0;
}