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
fn bracket(values: &[i32]) -> String {
    format!("[{}]", values.iter().map(i32::to_string).collect::<Vec<_>>().join(" "))
}
fn list_position(where_: &str, reference: usize, len: usize, insert: bool) -> Result<usize, String> {
    match where_ {
        "HEAD" => Ok(0),
        "TAIL" => Ok(if insert { len } else { len - 1 }),
        "BEFORE" if reference < len => {
            if insert { Ok(reference) }
            else if reference > 0 { Ok(reference - 1) }
            else { Err("no element before reference".into()) }
        },
        "AFTER" if reference < len => {
            if !insert && reference + 1 == len { Err("no element after reference".into()) }
            else { Ok(reference + 1) }
        },
        "BEFORE" | "AFTER" => Err("reference index out of range".into()),
        _ => Err("expected HEAD, TAIL, BEFORE or AFTER".into()),
    }
}
fn list_insert(c: &mut Container, index: usize, v: i32) -> Result<(), String> {
    match c {
        Container::Forward(x) => x.insert(index, v),
        Container::Doubly(x) => x.insert(index, v),
        _ => Err("not a list".into()),
    }
}
fn list_erase(c: &mut Container, index: usize) -> Result<i32, String> {
    match c {
        Container::Forward(x) => x.erase(index),
        Container::Doubly(x) => x.erase(index),
        _ => Err("not a list".into()),
    }
}
fn list_len(c: &Container) -> usize {
    match c { Container::Forward(x) => x.len(), Container::Doubly(x) => x.len(), _ => 0 }
}

pub fn run(db: &mut Database, line: &str) -> Result<(String, bool), String> {
    let args: Vec<&str> = line.split_whitespace().collect();
    if args.is_empty() { return Ok((String::new(), false)); }
    let op = args[0].to_ascii_uppercase();
    if op == "HELP" {
        count(&args, 1)?;
        return Ok(("CREATE type name | PRINT name | MPUSH/MINSERT/MGET/MSET/MDEL/MLEN/MREAD | F/L PUSH/INSERT/DEL/REMOVE/FIND/GET/READ | SPUSH/SPOP/SGET/SREAD | QPUSH/QPOP/QGET/QREAD | EXIT".into(), false));
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
    let name = required(&args, 1)?;
    let container = db.get_mut(name).ok_or_else(|| format!("structure not found: {name}"))?;
    if op == "PRINT" {
        count(&args, 2)?;
        return Ok((bracket(&container.values()), false));
    }
    if !op.starts_with(container.kind()) { return Err("command does not match structure type".into()); }

    match container {
        Container::Array(x) => match op.as_str() {
            "MPUSH" => { count(&args, 3)?; x.push(parse_value(args[2])?); Ok(("OK".into(), true)) },
            "MINSERT" => { count(&args, 4)?; let i = parse_index(args[2])?;
                let v = parse_value(args[3])?; x.insert(i,v)?; Ok(("OK".into(), true)) },
            "MSET" => { count(&args, 4)?; let i = parse_index(args[2])?;
                let v = parse_value(args[3])?; x.set(i,v)?; Ok(("OK".into(), true)) },
            "MDEL" => { count(&args, 3)?; Ok((x.erase(parse_index(args[2])?)?.to_string(), true)) },
            "MGET" => { count(&args, 3)?; Ok((x.get(parse_index(args[2])?)?.to_string(), false)) },
            "MLEN" => { count(&args, 2)?; Ok((x.len().to_string(), false)) },
            "MREAD" => { count(&args, 2)?; Ok((bracket(&x.values()), false)) },
            _ => Err(format!("unknown command: {op}")),
        },
        Container::Forward(_) | Container::Doubly(_) => {
            let prefix = container.kind();
            let len = list_len(container);
            if op == format!("{prefix}PUSH") {
                count(&args, 3)?;
                let v = parse_value(args[2])?;
                list_insert(container, len, v)?;
                return Ok(("OK".into(), true));
            }
            if op == format!("{prefix}INSERT") || op == format!("{prefix}DEL") {
                let insert = op.ends_with("INSERT");
                let where_ = required(&args, 2)?.to_ascii_uppercase();
                let need_ref = where_ == "BEFORE" || where_ == "AFTER";
                count(&args, 3 + usize::from(need_ref) + usize::from(insert))?;
                let mut offset = 3;
                let reference = if need_ref { offset += 1; parse_index(args[3])? } else { 0 };
                let value = if insert { Some(parse_value(args[offset])?) } else { None };
                if !insert && len == 0 { return Err("list is empty".into()); }
                let i = list_position(&where_, reference, len, insert)?;
                if let Some(v) = value { list_insert(container, i, v)?; return Ok(("OK".into(), true)); }
                return Ok((list_erase(container, i)?.to_string(), true));
            }
            if op == format!("{prefix}REMOVE") {
                count(&args, 3)?;
                let v = parse_value(args[2])?;
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
                let v = parse_value(args[2])?;
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
                return Ok((v.to_string(), false));
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
            "SPUSH" => { count(&args,3)?; x.push(parse_value(args[2])?); Ok(("OK".into(),true)) },
            "SPOP" => { count(&args,2)?; Ok((x.pop()?.to_string(),true)) },
            "SGET" => { count(&args,2)?; Ok((x.top()?.to_string(),false)) },
            "SREAD" => { count(&args,2)?; Ok((bracket(&x.values()),false)) },
            _ => Err(format!("unknown command: {op}")),
        },
        Container::Queue(x) => match op.as_str() {
            "QPUSH" => { count(&args,3)?; x.push(parse_value(args[2])?); Ok(("OK".into(),true)) },
            "QPOP" => { count(&args,2)?; Ok((x.pop()?.to_string(),true)) },
            "QGET" => { count(&args,2)?; Ok((x.front()?.to_string(),false)) },
            "QREAD" => { count(&args,2)?; Ok((bracket(&x.values()),false)) },
            _ => Err(format!("unknown command: {op}")),
        },
    }
}
