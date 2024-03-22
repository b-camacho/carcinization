# RIIR: Grocery checkout
With two rust learning sessions under your belt, you are ready for the crustacean rite of passage: Rewriting It In Rust.

# Goal
Under `cpp/` you'll find a fully working C++ app for scanning barcodes and receipts.
It uses two data files.

An inventory file:
```
# inventory.txt
i 1848 milk # new item, barcode = 1848, name = milk
p 1848 350 # item price, for barcode 1848, price = 350 cents
d 1848 325 # discounted price for barcode 1848
i 30 butter
p 30 499 # no discounts on butter :(
```
and
```
# cart.txt
I 1848 # first item in cart is barcode Milk
I 30 # second item is butter
I 1848 # third item is another carton of milk
D 10 # user scanned a discount code (ignore the code)
```

Calling your program like this:
`register inventory.txt cart.txt`
should print a receipt that looks something like this:
```
//// Receipt ////
Butter (1): $4.99
Milk (2): $6.50
Total: $11.49, Savings: $0.50
```
Notice it even prints our discount savings!

