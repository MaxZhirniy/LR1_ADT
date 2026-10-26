use crate::storage::{Container, Database};

fn parse_value(text: &str) -> Result<i32, String> {
    text.parse::<i32>().map_err(|_| format!("invalid integer: {text}"))
}
fn parse_index(text: &str) -> Result<usize, String> {
    let n = parse_value(text)?;
    usize::try_from(n).map_err(|_| "negative index".into())
}
fn required<'a>(args: &'a [&str], i: usize) -> Result<&'a str, String> {
    args.get(i).copied().ok_or_else(|| "missing command argument".into())
}
fn count(args: &[&str], n: usize) -> Result<(), String> {
    if args.len() == n { Ok(()) } else { Err("invalid number of command arguments".into()) }
}
fn bracket(values: &[String]) -> String {
    format!("[{}]", values.join(" "))
}
fn list_insert(c: &mut Container, where_: &str, target: &str, v: String) -> Result<(), String> {
    match c {
        Container::Forward(x) => x.insert(where_, target, v),
        Container::Doubly(x) => x.insert(where_, target, v),
        _ => Err("not a list".into()),
    }
}
fn list_erase(c: &mut Container, where_: &str, target: &str) -> Result<String, String> {
    match c {
        Container::Forward(x) => x.erase(where_, target),
        Container::Doubly(x) => x.erase(where_, target),
        _ => Err("not a list".into()),
    }
}

pub fn run(db: &mut Database, line: &str) -> Result<(String, bool), String> {
    let args: Vec<&str> = line.split_whitespace().collect();
    if args.is_empty() { return Ok((String::new(), false)); }
    let op = args[0].to_ascii_uppercase();
    if op == "HELP" {
        count(&args, 1)?;
        return Ok(("CREATE type name | DROP name | PRINT name | MPUSH/MINSERT/MGET/MSET/MDEL/MLEN/MREAD | F/L PUSH/INSERT/DEL/REMOVE/FIND/GET/READ | SPUSH/SPOP/SGET/SREAD | QPUSH/QPOP/QGET/QREAD | EXIT".into(), false));
    }
    if op == "CREATE" {
        count(&args, 3)?;
        let kind = required(&args, 1)?.to_ascii_uppercase();
        if kind.len() != 1 { return Err("type must be M/F/L/S/Q".into()); }
        let name = args[2];
        if db.contains_key(name) { return Err("name already exists".into()); }
        let container = Container::new(kind.chars().next().unwrap())?;
        db.insert(name.into(), container);
        return Ok(("OK".into(), true));
    }
    if op == "DROP" {
        count(&args, 2)?;
        let name = required(&args, 1)?;
        if db.remove(name).is_none() { return Err(format!("structure not found: {name}")); }
        return Ok(("OK".into(), true));
    }
    let name = required(&args, 1)?;
    let container = db.get_mut(name).ok_or_else(|| format!("structure not found: {name}"))?;
    if op == "PRINT" {
        count(&args, 2)?;
        return Ok((bracket(&container.values()), false));
    }
    if !op.starts_with(container.kind()) { return Err("command does not match structure type".into()); }

    match container {
        Container::Array(x) => match op.as_str() {
            "MPUSH" => { count(&args, 3)?; x.push(args[2].to_string()); Ok(("OK".into(), true)) },
            "MINSERT" => { count(&args, 4)?; let i = parse_index(args[2])?;
                let v = args[3].to_string(); x.insert(i,v)?; Ok(("OK".into(), true)) },
            "MSET" => { count(&args, 4)?; let i = parse_index(args[2])?;
                let v = args[3].to_string(); x.set(i,v)?; Ok(("OK".into(), true)) },
            "MDEL" => { count(&args, 3)?; Ok((x.erase(parse_index(args[2])?)?, true)) },
            "MGET" => { count(&args, 3)?; Ok((x.get(parse_index(args[2])?)?, false)) },
            "MLEN" => { count(&args, 2)?; Ok((x.len().to_string(), false)) },
            "MREAD" => { count(&args, 2)?; Ok((bracket(&x.values()), false)) },
            _ => Err(format!("unknown command: {op}")),
        },
        Container::Forward(_) | Container::Doubly(_) => {
            let prefix = container.kind();
            if op == format!("{prefix}PUSH") {
                count(&args, 3)?;
                let v = args[2].to_string();
                match container {
                    Container::Forward(x) => x.push_back(v),
                    Container::Doubly(x) => x.push_back(v),
                    _ => unreachable!(),
                }
                return Ok(("OK".into(), true));
            }
            if op == format!("{prefix}INSERT") || op == format!("{prefix}DEL") {
                let insert = op.ends_with("INSERT");
                let where_ = required(&args, 2)?.to_ascii_uppercase();
                let need_ref = where_ == "BEFORE" || where_ == "AFTER";
                count(&args, 3 + usize::from(need_ref) + usize::from(insert))?;
                let target = if need_ref { args[3] } else { "" };
                if insert {
                    let value = args[3 + usize::from(need_ref)].to_string();
                    list_insert(container, &where_, target, value)?;
                    return Ok(("OK".into(), true));
                }
                return Ok((list_erase(container, &where_, target)?, true));
            }
            if op == format!("{prefix}REMOVE") {
                count(&args, 3)?;
                let v = args[2];
                let removed = match container {
                    Container::Forward(x) => x.remove_value(v),
                    Container::Doubly(x) => x.remove_value(v),
                    _ => false,
                };
                let answer = if removed { "TRUE" } else { "FALSE" };
                return Ok((answer.to_string(), removed));
            }
            if op == format!("{prefix}FIND") {
                count(&args, 3)?;
                let v = args[2];
                let pos = match container {
                    Container::Forward(x) => x.find(v),
                    Container::Doubly(x) => x.find(v),
                    _ => None,
                };
                return Ok((pos.map(|x| x.to_string()).unwrap_or_else(|| "-1".into()), false));
            }
            if op == format!("{prefix}GET") {
                count(&args, 3)?;
                let i = parse_index(args[2])?;
                let v = match container {
                    Container::Forward(x) => x.get(i)?,
                    Container::Doubly(x) => x.get(i)?,
                    _ => unreachable!(),
                };
                return Ok((v, false));
            }
            if op == format!("{prefix}READ") {
                if args.len() > 3 { return Err("invalid number of command arguments".into()); }
                let mode = args.get(2).copied().unwrap_or("FORWARD").to_ascii_uppercase();
                if mode != "FORWARD" && mode != "REVERSE" { return Err("read mode: FORWARD or REVERSE".into()); }
                let values = match container {
                    Container::Forward(x) => x.values(mode == "REVERSE"),
                    Container::Doubly(x) => x.values(mode == "REVERSE"),
                    _ => unreachable!(),
                };
                return Ok((bracket(&values), false));
            }
            Err(format!("unknown command: {op}"))
        }
        Container::Stack(x) => match op.as_str() {
            "SPUSH" => { count(&args,3)?; x.push(args[2].to_string()); Ok(("OK".into(),true)) },
            "SPOP" => { count(&args,2)?; Ok((x.pop()?,true)) },
            "SGET" => { count(&args,2)?; Ok((x.top()?,false)) },
            "SREAD" => { count(&args,2)?; Ok((bracket(&x.values()),false)) },
            _ => Err(format!("unknown command: {op}")),
        },
        Container::Queue(x) => match op.as_str() {
            "QPUSH" => { count(&args,3)?; x.push(args[2].to_string()); Ok(("OK".into(),true)) },
            "QPOP" => { count(&args,2)?; Ok((x.pop()?,true)) },
            "QGET" => { count(&args,2)?; Ok((x.front()?,false)) },
            "QREAD" => { count(&args,2)?; Ok((bracket(&x.values()),false)) },
            _ => Err(format!("unknown command: {op}")),
        },
    }
}
