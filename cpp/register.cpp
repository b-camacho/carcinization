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

    const auto inventory = parseInventory(ifs);
    printInventory(inventory);
    return 0;
}

