use std::collections::HashMap;
struct Args {
    inventory_path: std::path::PathBuf
}

struct Listing {
    name: String,
    price: i32
}

type Barcode = i32;

type Inventory = HashMap<Barcode, Listing>;

struct ReceiptLine<'a> {
    quantity: i32,
    what: &'a Listing
}

type Receipt<'a> = HashMap<Barcode, ReceiptLine<'a>>;

impl Receipt {
    pub fn new() {
        HashMap::new()
    }
}
impl Display for Receipt {
    pub fn display(/*...*/) {
        //todo: formatting here


    }

}

fn scan_item<'a>(barcode: Barcode, receipt: &mut Receipt<'a>,  inventory: &'a Inventory) -> Result<(), String> {
    if let Some(r_line) = receipt.get_mut(&barcode) {
        r_line.quantity += 1;
        return Ok(())
    }

    if let Some(what) = inventory.get(&barcode) {
        receipt.insert(barcode.clone(), ReceiptLine{ quantity: 1, what} );
        Ok(())
    } else {
        Err(format!("no item with barcode: {}", barcode))
    }
}

fn parse_inventory(text: &str) -> Option<Inventory> {
    unimplemented!("serde json parsing here")
}

fn parse_barcodes(text: &str) -> Option<Vec<i32>> {
    unimplemented!("serde json parsing barcodes")

}

fn parse_args(argv: &Vec<&str>) -> Option<Args> {
    if argv.len() != 3 || argv[1] != "--inventory" {
        //todo: import log facade
        //log::error!("");
        None
    } else {
        Some ( Args { inventory_path: argv[2].to_owned().into() } )
    }
}

