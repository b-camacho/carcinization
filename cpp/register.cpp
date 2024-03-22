#include <unordered_map>

struct Listing {
    std::string name;
    int price;
};

using Barcode = int;

std::unordered_map<Barcode, Listing> inventory;

// _Wouldn't rehash or invalidate any of the Listing* in receipts.
// We *can't* do this in Rust while we have a Receipt referencing inventory
// because we'd be mutably borrowing the inventory and the listings would be
// immutably borrowing it.
// But maybe we *want* that! Would be fucky to change prices *while* checking out.
inventory[42].price += 50;

struct ReceiptLine {
    int quantity;
    Listing* what;
};

std::unordered_map<Barcode, ReceiptLine> receipt;
