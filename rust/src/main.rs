use std::io::Read;

mod lib;

fn main() {
    let os_args = std::env::args().collect::<Vec<String>>();
    
    let args = parse_args(&os_args)
        .expect("usage: echo <barcodes json> | receipt --inventory /path/to/inventory.json");

    let inventory_text = std::io::read_to_string(args.inventory_path.read()).expect("inventory: no file");
    let inventory = parse_inventory(&inventory_text).expect("inventory: no parse");

    let mut barcode_text = Vec::new();
    std::io::stdin().read_to_end(&mut barcode_text);
    let barcodes = parse_barcodes(barcode_text);

    let mut receipt = Receipt::new();
    for barcode in barcodes {
        scan_item(barcode, &mut receipt, &inventory)
    }

    println!("{receipt}");


    println!("Hello, world!");
    
}

