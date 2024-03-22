#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <variant>

struct Listing {
    std::string name;
    int price;
    std::optional<int> discountPrice;
};

using Barcode = int;

using Inventory = std::map<Barcode, Listing>;

void printInventory(const Inventory& i)
{
    for (const auto& kv : i) {
        const auto& l = kv.second;
        printf("%d: %s %.2f", kv.first, l.name.c_str(), l.price / 100.0f);
        if (l.discountPrice) printf(" (%.2f)", *l.discountPrice / 100.0f);
        printf("\n");
    }
}

struct ItemName {
    int id;
    std::string name;
};

struct ItemPrice {
    int id;
    int price;
};

struct ItemDiscountPrice {
    int id;
    int discountPrice;
};

using ParsedLine = std::variant<ItemName, ItemPrice, ItemDiscountPrice>;

ParsedLine parseInventoryLine(const char* l)
{
    char lineKind = 0;
    int id = 0;
    char* payload = nullptr;
    assert(sscanf(l, "%c %d %ms", &lineKind, &id, &payload) == 3);

    ParsedLine p;

    switch (lineKind) {
        case 'I':
            p = ItemName { .id = id, .name = payload };
            break;

        case 'P': {
            int price = 0;
            assert(sscanf(payload, "%d", &price) == 1);
            p = ItemPrice { .id = id, .price = price };
            break;
        }

        case 'D': {
            int dis = 0;
            assert(sscanf(payload, "%d", &dis) == 1);
            p = ItemDiscountPrice { .id = id, .discountPrice = dis };
            break;
        }

        default:
            fprintf(stderr, "Bogus inventory line: %s\n", l);
            abort();
    }

    free(payload);
    return p;
}

// pain
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

Inventory parseInventory(std::ifstream& f) {
    Inventory inventory;
    std::string l;
    while (std::getline(f, l)) {
        auto parsed = parseInventoryLine(l.c_str());

        std::visit(overloaded(
                [&](ItemName&& in) {
                    inventory[in.id].name = std::move(in.name);
                },
                [&](ItemPrice&& ip) {
                    inventory[ip.id].price = ip.price;
                },
                [&](ItemDiscountPrice&& idp) {
                    inventory[idp.id].discountPrice = idp.discountPrice;
                }
            ),
            std::move(parsed));
    }
    return inventory;
}

struct ReceiptLine {
    const Listing* what = nullptr;
    int quantity = 0;
};

struct Receipt {
    std::map<Barcode, ReceiptLine> items;
    bool discountMember = false;
};

void printReceipt(const Receipt& r) {
    int total = 0;
    int savings = 0;
    for (const auto& kv : r.items) {
        const auto& item = kv.second;
        const auto& listing = *item.what;
        int price = 0;
        if (r.discountMember) {
            price = listing.discountPrice.value_or(listing.price);
        } else {
            price = listing.price;
        }
        printf("%d %s: %.2f\n", item.quantity, listing.name.c_str(), price / 100.0f);
        total += price;
        if (r.discountMember && listing.discountPrice) {
            savings += listing.price - *listing.discountPrice;
        }
    }
    printf("Total: %.2f\n", total / 100.0f);
    printf("Savings: %.2f\n", savings / 100.0f);
}

Receipt parseCart(std::ifstream& f, const Inventory& inventory) {
    Receipt receipt;
    std::string l;
    while (std::getline(f, l)) {
        char code = 0;
        int id = 0;
        assert(sscanf(l.c_str(), "%c %d", &code, &id) == 2);
        switch (code) {
            case 'I': {
                auto& receiptLine = receipt.items[id];
                if (receiptLine.what == nullptr) {
                    // This is the first time we're mentioning this item;
                    // point its "what" pointer at the inventory entry.
                    const auto invEntry = inventory.find(id);
                    if (invEntry == inventory.end()) {
                        fprintf(stderr, "Can't find item %d!\n", id);
                        abort();
                    }
                    receiptLine.what = &invEntry->second;
                }
                ++receiptLine.quantity;
                break;
            }
            case 'D':
                receipt.discountMember = true;
                break;

            default:
                fprintf(stderr, "Bogus cart line: %s\n", l.c_str());
                abort();
        }
    }
    return receipt;
}

// _Wouldn't rehash or invalidate any of the Listing* in receipts.
// We *can't* do this in Rust while we have a Receipt referencing inventory
// because we'd be mutably borrowing the inventory and the listings would be
// immutably borrowing it.
// But maybe we *want* that! Would be fucky to change prices *while* checking out.
/*
inventory[42].price += 50;

struct ReceiptLine {
    int quantity;
    Listing* what;
};

std::unordered_map<Barcode, ReceiptLine> receipt;
*/

int main(int argc, const char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s inventory.txt cart.txt\n", argv[0]);
        return 1;
    }

    std::ifstream ifs(argv[1]);
    if (!ifs) {
        fprintf(stderr, "Couldn't open %s\n", argv[1]);
        return 1;
    }

    auto inventory = parseInventory(ifs);
    printInventory(inventory);

    std::ifstream cfs(argv[2]);
    if (!cfs) {
        fprintf(stderr, "Couldn't open %s\n", argv[2]);
        return 1;
    }

    const auto receipt = parseCart(cfs, inventory);
    printf("\n");
    printReceipt(receipt);

    // Inflation lol
    for (auto& kv : inventory) {
        auto& v = kv.second;
        v.price *= 12;
        v.price /= 10;
        if (v.discountPrice) { // It comes for us all
            *v.discountPrice *= 12;
            *v.discountPrice /= 10;
        }
    }

    // const is fun in C++ - we have a const reference to an inventory listing,
    // and yet they all changed!
    // And our data structure (the receipt) that points to it changes as a result!
    printf("\nThis ain't right - the receipt changed!\n");
    printReceipt(receipt);

    return 0;
}

