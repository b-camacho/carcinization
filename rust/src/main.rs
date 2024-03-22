use rcpt::{
    parse_args, parse_barcodes, parse_inventory, scan_item, show_receipt, Args, Barcode, Inventory,
    Listing, Receipt, ReceiptLine,
};
use std::io::Read;

fn main() {
    let usage = "Usage: receipt /path/to/inventory /path/to/cart";
    let os_args = std::env::args().collect::<Vec<String>>();

    let args = parse_args(&os_args).expect(usage);

    let inventory_text = std::fs::read_to_string(args.inventory_path).expect("inventory: no file");
    let inventory = parse_inventory(&inventory_text).expect("inventory: no parse");

    let mut barcode_text = std::fs::read_to_string(args.cart_path).expect("cart: no file");
    let (barcodes, is_discount) = parse_barcodes(&barcode_text).expect("cart: no parse");

    let mut receipt = Receipt::new();
    receipt.disc = is_discount;
    for barcode in barcodes {
        scan_item(barcode, &mut receipt, &inventory).expect("bad scan");
    }
    show_receipt(&receipt);
}
