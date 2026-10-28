
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <memory>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

// ==================== PRODUCT BASE CLASS ====================

class Product {
private:
    int id;
    string name;
    double price;
    int stock;

public:
    Product(int id, string name, double price, int stock)
        : id(id), name(name), price(price), stock(stock) {
        if (price < 0 || stock < 0) {
            throw invalid_argument("Price and stock cannot be negative.");
        }
    }

    virtual ~Product() = default;

    int getId() const {
        return id;
    }

    string getName() const {
        return name;
    }

    double getPrice() const {
        return price;
    }

    int getStock() const {
        return stock;
    }

    void reduceStock(int quantity) {
        if (quantity <= 0 || quantity > stock) {
            throw runtime_error("Invalid quantity or insufficient stock.");
        }

        stock -= quantity;
    }

    void addStock(int quantity) {
        if (quantity <= 0) {
            throw invalid_argument("Quantity must be positive.");
        }

        stock += quantity;
    }

    virtual string getCategory() const = 0;

    virtual double getDiscount() const {
        return 0.0;
    }

    double getFinalPrice() const {
        return price * (1.0 - getDiscount());
    }

    virtual void display() const {
        cout << left << setw(6) << id
             << setw(24) << name
             << setw(16) << getCategory()
             << setw(12) << fixed << setprecision(2) << price
             << setw(12) << getFinalPrice()
             << setw(8) << stock << '\n';
    }

    virtual string serialize() const = 0;
};


// ==================== ELECTRONICS CLASS ====================

class Electronics : public Product {
public:
    Electronics(int id, string name, double price, int stock)
        : Product(id, name, price, stock) {}

    string getCategory() const override {
        return "Electronics";
    }

    double getDiscount() const override {
        return 0.10; // 10% discount
    }

    string serialize() const override {
        ostringstream out;
        out << "E|" << getId() << "|"
            << getName() << "|"
            << getPrice() << "|"
            << getStock();

        return out.str();
    }
};


// ==================== CLOTHING CLASS ====================

class Clothing : public Product {
public:
    Clothing(int id, string name, double price, int stock)
        : Product(id, name, price, stock) {}

    string getCategory() const override {
        return "Clothing";
    }

    double getDiscount() const override {
        return 0.15; // 15% discount
    }

    string serialize() const override {
        ostringstream out;
        out << "C|" << getId() << "|"
            << getName() << "|"
            << getPrice() << "|"
            << getStock();

        return out.str();
    }
};


// ==================== CUSTOMER CLASS ====================

class Customer {
private:
    int id;
    string name;
    string preferredCategory;
    bool premiumMember;

public:
    Customer(int id, string name,
             string preferredCategory, bool premiumMember)
        : id(id), name(name),
          preferredCategory(preferredCategory),
          premiumMember(premiumMember) {}

    int getId() const {
        return id;
    }

    string getName() const {
        return name;
    }

    string getPreferredCategory() const {
        return preferredCategory;
    }

    bool isPremium() const {
        return premiumMember;
    }
};


// ==================== CART ITEM ====================

struct CartItem {
    int productId;
    int quantity;
};


// ==================== SHOPPING CART ====================

class Cart {
private:
    vector<CartItem> items;

public:
    void addItem(int productId, int quantity) {
        if (quantity <= 0) {
            throw invalid_argument("Quantity must be positive.");
        }

        for (auto &item : items) {
            if (item.productId == productId) {
                item.quantity += quantity;
                return;
            }
        }

        items.push_back({productId, quantity});
    }

    void clear() {
        items.clear();
    }

    bool isEmpty() const {
        return items.empty();
    }

    const vector<CartItem>& getItems() const {
        return items;
    }

    void display() const {
        if (items.empty()) {
            cout << "\nYour cart is empty.\n";
            return;
        }

        cout << "\n========== YOUR CART ==========\n";

        for (const auto &item : items) {
            cout << "Product ID: " << item.productId
                 << " | Quantity: " << item.quantity << '\n';
        }
    }
};


// ==================== ORDER CLASS ====================

class Order {
private:
    int orderId;
    int customerId;
    double total;

public:
    Order(int orderId, int customerId, double total)
        : orderId(orderId), customerId(customerId), total(total) {}

    void save(ofstream &file) const {
        file << orderId << "|"
             << customerId << "|"
             << fixed << setprecision(2) << total << '\n';
    }
};


// ==================== SHOPSPHERE APPLICATION ====================

class ShopSphere {
private:
    vector<unique_ptr<Product>> products;
    map<int, double> salesByProduct;

    Customer customer{1, "Guest", "Electronics", false};
    Cart cart;

    int nextOrderId = 1001;
    double totalRevenue = 0.0;

    // Find product by ID.
    Product* findProduct(int id) {
        for (auto &product : products) {
            if (product->getId() == id) {
                return product.get();
            }
        }

        return nullptr;
    }

    // Load initial products when no saved inventory exists.
    void addDefaultProducts() {
        products.push_back(
            make_unique<Electronics>(101, "Wireless Headphones", 2500, 10)
        );

        products.push_back(
            make_unique<Electronics>(102, "Smart Watch", 3500, 8)
        );

        products.push_back(
            make_unique<Clothing>(201, "Casual Hoodie", 1200, 15)
        );

        products.push_back(
            make_unique<Clothing>(202, "Classic T-Shirt", 600, 20)
        );
    }

    // Save inventory to a file.
    void saveProducts() const {
        ofstream file("products.txt");

        if (!file) {
            throw runtime_error("Unable to save products.txt");
        }

        file << fixed << setprecision(2);

        for (const auto &product : products) {
            file << product->serialize() << '\n';
        }
    }

    // Load inventory from a file.
    void loadProducts() {
        ifstream file("products.txt");

        if (!file) {
            addDefaultProducts();
            saveProducts();
            return;
        }

        string line;

        while (getline(file, line)) {
            try {
                stringstream ss(line);

                string type, name, priceText, stockText;
                int id;

                if (!getline(ss, type, '|')) continue;

                string idText;

                if (!getline(ss, idText, '|') ||
                    !getline(ss, name, '|') ||
                    !getline(ss, priceText, '|') ||
                    !getline(ss, stockText)) {
                    continue;
                }

                id = stoi(idText);
                double price = stod(priceText);
                int stock = stoi(stockText);

                if (type == "E") {
                    products.push_back(
                        make_unique<Electronics>(
                            id, name, price, stock
                        )
                    );
                } else if (type == "C") {
                    products.push_back(
                        make_unique<Clothing>(
                            id, name, price, stock
                        )
                    );
                }
            } catch (const exception &) {
                cerr << "Skipping an invalid product record.\n";
            }
        }

        if (products.empty()) {
            addDefaultProducts();
            saveProducts();
        }
    }

    // Show all products.
    void showProducts() const {
        cout << "\n================ PRODUCT CATALOG ================\n";

        cout << left << setw(6) << "ID"
             << setw(24) << "Product"
             << setw(16) << "Category"
             << setw(12) << "Price"
             << setw(12) << "Sale Price"
             << setw(8) << "Stock" << '\n';

        cout << string(78, '-') << '\n';

        for (const auto &product : products) {
            product->display();
        }
    }

    // Smart recommendation using customer preference.
    void recommendProducts() const {
        cout << "\n========== SMART RECOMMENDATIONS ==========\n";
        cout << "Recommended for category: "
             << customer.getPreferredCategory() << "\n\n";

        bool found = false;

        for (const auto &product : products) {
            if (product->getCategory() ==
                    customer.getPreferredCategory()
                && product->getStock() > 0) {

                product->display();
                found = true;
            }
        }

        if (!found) {
            cout << "No products available in your preferred category.\n";
        }

        cout << "\nNote: Recommendations are rule-based, not machine learning.\n";
    }

    // Add a product to the cart.
    void addToCart() {
        int id, quantity;

        cout << "Enter product ID: ";
        cin >> id;

        cout << "Enter quantity: ";
        cin >> quantity;

        Product *product = findProduct(id);

        if (product == nullptr) {
            cout << "Product not found.\n";
            return;
        }

        if (quantity <= 0) {
            cout << "Quantity must be positive.\n";
            return;
        }

        int alreadyInCart = 0;

        for (const auto &item : cart.getItems()) {
            if (item.productId == id) {
                alreadyInCart = item.quantity;
                break;
            }
        }

        if (quantity + alreadyInCart > product->getStock()) {
            cout << "Not enough stock available.\n";
            return;
        }

        cart.addItem(id, quantity);
        cout << "Product added to cart successfully.\n";
    }

    // Function overloading: total without premium discount.
    double calculateTotal(const Cart &shoppingCart) {
        double total = 0.0;

        for (const auto &item : shoppingCart.getItems()) {
            Product *product = findProduct(item.productId);

            if (product != nullptr) {
                total += product->getFinalPrice() * item.quantity;
            }
        }

        return total;
    }

    // Function overloading: total with optional premium discount.
    double calculateTotal(const Cart &shoppingCart,
                          bool premiumMember) {
        double total = calculateTotal(shoppingCart);

        if (premiumMember) {
            total *= 0.95; // Additional 5% premium discount.
        }

        return total;
    }

    // Checkout and save order.
    void checkout() {
        if (cart.isEmpty()) {
            cout << "Your cart is empty.\n";
            return;
        }

        // Validate every item before changing inventory.
        for (const auto &item : cart.getItems()) {
            Product *product = findProduct(item.productId);

            if (product == nullptr ||
                item.quantity <= 0 ||
                item.quantity > product->getStock()) {
                cout << "Checkout failed. Please review your cart.\n";
                return;
            }
        }

        double subtotal = calculateTotal(cart);
        double total = calculateTotal(cart, customer.isPremium());

        cout << "\n========== CHECKOUT ==========\n";
        cout << "Customer: " << customer.getName() << '\n';
        cout << "Subtotal after product discounts: Rs. "
             << fixed << setprecision(2) << subtotal << '\n';

        if (customer.isPremium()) {
            cout << "Premium discount: 5%\n";
        }

        cout << "Amount payable: Rs. " << total << '\n';

        char confirm;

        cout << "Confirm purchase? (y/n): ";
        cin >> confirm;

        if (confirm != 'y' && confirm != 'Y') {
            cout << "Checkout cancelled.\n";
            return;
        }

        ofstream file("orders.txt", ios::app);

        if (!file) {
            cout << "Unable to save order. Inventory was not changed.\n";
            return;
        }

        // Save the order before modifying inventory.
        Order order(nextOrderId, customer.getId(), total);
        order.save(file);
        file.close();

        for (const auto &item : cart.getItems()) {
            Product *product = findProduct(item.productId);

            product->reduceStock(item.quantity);
            salesByProduct[item.productId] += item.quantity;
        }

        totalRevenue += total;

        cout << "\nOrder placed successfully!\n";
        cout << "Order ID: " << nextOrderId << '\n';

        ++nextOrderId;
        cart.clear();

        saveProducts();
    }

    // Sales and inventory analytics.
    void showAnalytics() {
        cout << "\n========== SHOPSPHERE ANALYTICS ==========\n";

        cout << "Revenue from this session: Rs. "
             << fixed << setprecision(2)
             << totalRevenue << '\n';

        cout << "\nInventory status:\n";

        for (const auto &product : products) {
            cout << product->getName()
                 << " | Stock: " << product->getStock();

            if (product->getStock() <= 3) {
                cout << " | LOW STOCK ALERT";
            }

            cout << '\n';
        }

        cout << "\nUnits sold during this session:\n";

        if (salesByProduct.empty()) {
            cout << "No completed sales in this session.\n";
        } else {
            for (const auto &sale : salesByProduct) {
                Product *product = findProduct(sale.first);

                if (product != nullptr) {
                    cout << product->getName()
                         << " | Units sold: " << sale.second << '\n';
                }
            }
        }
    }

    // Input validation for the main menu.
    int getMenuChoice() const {
        int choice;

        cout << "\nEnter your choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return -1;
        }

        return choice;
    }

public:
    ShopSphere() {
        loadProducts();
    }

    void run() {
        int choice;

        do {
            cout << "\n";
            cout << "============================================\n";
            cout << "          SHOPSPHERE AI 🛍️\n";
            cout << "    Smart E-Commerce Management System\n";
            cout << "============================================\n";
            cout << "1. View Products\n";
            cout << "2. Add Product to Cart\n";
            cout << "3. View Cart\n";
            cout << "4. Smart Recommendations\n";
            cout << "5. Checkout\n";
            cout << "6. Sales & Inventory Analytics\n";
            cout << "7. Exit\n";

            choice = getMenuChoice();

            try {
                switch (choice) {
                    case 1:
                        showProducts();
                        break;

                    case 2:
                        addToCart();
                        break;

                    case 3:
                        cart.display();
                        break;

                    case 4:
                        recommendProducts();
                        break;

                    case 5:
                        checkout();
                        break;

                    case 6:
                        showAnalytics();
                        break;

                    case 7:
                        saveProducts();
                        cout << "Thank you for shopping with ShopSphere!\n";
                        break;

                    default:
                        cout << "Invalid choice. Please try again.\n";
                }
            } catch (const exception &e) {
                cout << "Error: " << e.what() << '\n';
            }

        } while (choice != 7);
    }
};


// ==================== MAIN FUNCTION ====================

int main() {
    try {
        ShopSphere app;
        app.run();
    } catch (const exception &e) {
        cerr << "Application error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
