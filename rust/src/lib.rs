use std::collections::HashMap;
pub struct Args {
    pub inventory_path: std::path::PathBuf,
    pub cart_path: std::path::PathBuf,
}

#[derive(Clone, Debug)]
pub struct Listing {
    name: String,
    price: i32,
    price_disc: Option<i32>,
}

pub type Barcode = i32;

pub type Inventory = HashMap<Barcode, Listing>;

#[derive(Clone, Debug)]
pub struct ReceiptLine<'a> {
    quantity: i32,
    what: &'a Listing,
}

#[derive(Clone, Debug)]
pub struct Receipt<'a> {
    pub items: HashMap<Barcode, ReceiptLine<'a>>,
    pub disc: bool,
}
impl<'a> Receipt<'a> {
    pub fn new() -> Self {
        Receipt {
            items: HashMap::new(),
            disc: false,
        }
    }
}

pub fn show_receipt(receipt: &Receipt) {
    let is_disc = receipt.disc;
    let totals: Vec<(_, _, _, _)> = receipt
        .items
        .iter()
        .map(|(_, line)| {
            let price = if is_disc {
                line.what.price_disc.unwrap_or(line.what.price)
            } else {
                line.what.price
            };
            let subtotal = (line.quantity * price) as f64 / 100f64;
            let savings = (line.quantity * line.what.price) as f64 / 100f64 - subtotal;
            (line.quantity, line.what.name.clone(), subtotal, savings)
        })
        .collect();

    let mut actual_total = 0f64;
    let mut total_savings = 0f64;
    for total in &totals {
        actual_total += total.2;
        total_savings += total.3;
    }

    let lines: Vec<String> = totals
        .iter()
        .map(|(quantity, name, subtotal, _)| format!("{name} ({quantity}): ${subtotal:.2}"))
        .collect();
    println!("{}", lines.join("\n"));
    println!("Total: ${actual_total:.2}, Savings: ${total_savings:.2}")
}

pub fn scan_item<'a>(
    barcode: Barcode,
    receipt: &mut Receipt<'a>,
    inventory: &'a Inventory,
) -> Result<(), String> {
    if let Some(r_line) = receipt.items.get_mut(&barcode) {
        r_line.quantity += 1;
        return Ok(());
    }

    if let Some(what) = inventory.get(&barcode) {
        receipt
            .items
            .insert(barcode.clone(), ReceiptLine { quantity: 1, what });
        Ok(())
    } else {
        Err(format!("no item with barcode: {}", barcode))
    }
}

pub fn parse_inventory(text: &str) -> Option<Inventory> {
    let mut inv = Inventory::new();
    for line in text.trim().lines() {
        let fields: Vec<&str> = line.split(" ").collect();
        match fields[..] {
            ["I", barcode, name] => {
                inv.insert(
                    barcode.parse::<i32>().ok()?,
                    Listing {
                        name: name.to_owned(),
                        price: 0,
                        price_disc: None,
                    },
                );
            }
            ["P", barcode, price] => {
                inv.get_mut(&barcode.parse::<i32>().ok()?)?.price = price.parse::<i32>().ok()?
            }
            ["D", barcode, price_disc] => {
                inv.get_mut(&barcode.parse::<i32>().ok()?)?.price_disc =
                    Some(price_disc.parse::<i32>().ok()?);
            }
            _ => return None,
        };
    }
    Some(inv)
}

pub fn parse_barcodes(text: &str) -> Option<(Vec<i32>, bool)> {
    let mut barcodes = Vec::new();
    let mut is_discount = false;
    for line in text.trim().lines() {
        let fields: Vec<&str> = line.split(" ").collect();
        match fields[..] {
            ["I", barcode] => barcodes.push(barcode.parse::<i32>().ok()?),
            ["D", _] => is_discount = true,
            _ => return None,
        };
    }

    Some((barcodes, is_discount))
}

pub fn parse_args(argv: &Vec<String>) -> Option<Args> {
    if argv.len() != 3 {
        None
    } else {
        Some(Args {
            inventory_path: argv[1].to_owned().into(),
            cart_path: argv[2].to_owned().into(),
        })
    }
}
