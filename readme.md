# RIIR: Grocery checkout
With two rust learning sessions under your belt, you are ready for the crustacean rite of passage: Rewriting It In Rust.

# Goal
Under `cpp/` you'll find a fully working C++ app for scanning barcodes, and printing a receipt.
The app will also load a list of available items ahead of time.

For example, given the file:
```
# items.json
{"barcode": 0x1337, "name": "apple", "price": 75}
{"barcode": 0x7331, "name": "orange", "price": 50}
```
your program could be called as
```
echo "[0x1337, 0x7331, 0x7331]" | receipt --items items.json
```
and should print
```
{"total": "$1.75", "items": [
 { "qty": 1, "name": "apple", "cost": "$0.75"}, {"qty": 2, "name": "apple", "cost": "$1.00"}]
```


