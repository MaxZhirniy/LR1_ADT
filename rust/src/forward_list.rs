/// Односвязный список с явным владением узлами через Box.
struct Node {
    value: i32,
    next: Option<Box<Node>>,
}

#[derive(Default)]
pub struct ForwardList {
    head: Option<Box<Node>>,
    size: usize,
}

impl ForwardList {
    pub fn len(&self) -> usize { self.size }

    pub fn insert(&mut self, index: usize, value: i32) -> Result<(), String> {
        if index > self.size { return Err("index out of range".into()); }
        let mut link = &mut self.head;
        for _ in 0..index { link = &mut link.as_mut().unwrap().next; }
        let next = link.take();
        *link = Some(Box::new(Node { value, next }));
        self.size += 1;
        Ok(())
    }
    pub fn erase(&mut self, index: usize) -> Result<i32, String> {
        if index >= self.size { return Err("index out of range".into()); }
        let mut link = &mut self.head;
        for _ in 0..index { link = &mut link.as_mut().unwrap().next; }
        let mut removed = link.take().unwrap();
        *link = removed.next.take();
        self.size -= 1;
        Ok(removed.value)
    }
    pub fn get(&self, index: usize) -> Result<i32, String> {
        if index >= self.size { return Err("index out of range".into()); }
        let mut node = self.head.as_deref();
        for _ in 0..index { node = node.unwrap().next.as_deref(); }
        Ok(node.unwrap().value)
    }
    pub fn find(&self, value: i32) -> Option<usize> {
        let mut node = self.head.as_deref();
        let mut index = 0;
        while let Some(item) = node {
            if item.value == value { return Some(index); }
            index += 1;
            node = item.next.as_deref();
        }
        None
    }
    pub fn remove_value(&mut self, value: i32) -> bool {
        match self.find(value) { Some(index) => self.erase(index).is_ok(), None => false }
    }
    pub fn values(&self, reverse: bool) -> Vec<i32> {
        let mut result = Vec::with_capacity(self.size);
        let mut current = self.head.as_deref();
        while let Some(item) = current {
            result.push(item.value);
            current = item.next.as_deref();
        }
        if reverse { result.reverse(); }
        result
    }
}
