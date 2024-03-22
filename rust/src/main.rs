use rcpt::{checkout, inflation, parse_args, parse_barcodes, parse_inventory, show_receipt};

fn main() {
    let usage = "Usage: receipt /path/to/inventory /path/to/cart";
    let os_args = std::env::args().collect::<Vec<String>>();

    let args = parse_args(&os_args).expect(usage);

    let inventory_text = std::fs::read_to_string(args.inventory_path).expect("inventory: no file");
    let mut inventory = parse_inventory(&inventory_text).expect("inventory: no parse");

    let barcode_text = std::fs::read_to_string(args.cart_path).expect("cart: no file");
    let (barcodes, is_discount) = parse_barcodes(&barcode_text).expect("cart: no parse");

    let receipt = checkout(&barcodes, is_discount, &inventory);

    show_receipt(&receipt);

    // can't mutate `inventory`, because `receipt` holds references
    // the compiler error is very helpful! uncomment this line to see
    // inflation(&mut inventory);

    // once we drop receipt, nobody can access it anymore
    // now we can mutate `inventory`
    std::mem::drop(receipt);
    inflation(&mut inventory);
    let receipt_2024 = checkout(&barcodes, is_discount, &inventory);
    show_receipt(&receipt_2024);
}
