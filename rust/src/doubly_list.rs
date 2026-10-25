/// Двусвязный список на индексном пуле узлов.
/// Индексы пула выступают вместо сырых указателей: нет unsafe,
/// но каждый активный узел содержит независимые prev и next.
struct Node {
    value: i32,
    prev: Option<usize>,
    next: Option<usize>,
}

#[derive(Default)]
pub struct DoublyList {
    nodes: Vec<Option<Node>>,
    free: Vec<usize>,
    head: Option<usize>,
    tail: Option<usize>,
    size: usize,
}

impl DoublyList {
    pub fn len(&self) -> usize { self.size }
    fn allocate(&mut self, node: Node) -> usize {
        if let Some(i) = self.free.pop() { self.nodes[i] = Some(node); i }
        else { self.nodes.push(Some(node)); self.nodes.len() - 1 }
    }
    fn node_at(&self, index: usize) -> Result<usize, String> {
        if index >= self.size { return Err("index out of range".into()); }
        if index <= self.size / 2 {
            let mut id = self.head.unwrap();
            for _ in 0..index { id = self.nodes[id].as_ref().unwrap().next.unwrap(); }
            Ok(id)
        } else {
            let mut id = self.tail.unwrap();
            for _ in 0..(self.size - index - 1) {
                id = self.nodes[id].as_ref().unwrap().prev.unwrap();
            }
            Ok(id)
        }
    }
    pub fn insert(&mut self, index: usize, value: i32) -> Result<(), String> {
        if index > self.size { return Err("index out of range".into()); }
        let next = if index == self.size { None } else { Some(self.node_at(index)?) };
        let prev = match next { Some(n) => self.nodes[n].as_ref().unwrap().prev, None => self.tail };
        let id = self.allocate(Node { value, prev, next });
        if let Some(p) = prev { self.nodes[p].as_mut().unwrap().next = Some(id); }
        else { self.head = Some(id); }
        if let Some(n) = next { self.nodes[n].as_mut().unwrap().prev = Some(id); }
        else { self.tail = Some(id); }
        self.size += 1;
        Ok(())
    }
    pub fn erase(&mut self, index: usize) -> Result<i32, String> {
        let id = self.node_at(index)?;
        let node = self.nodes[id].take().unwrap();
        if let Some(p) = node.prev { self.nodes[p].as_mut().unwrap().next = node.next; }
        else { self.head = node.next; }
        if let Some(n) = node.next { self.nodes[n].as_mut().unwrap().prev = node.prev; }
        else { self.tail = node.prev; }
        self.free.push(id);
        self.size -= 1;
        Ok(node.value)
    }
    pub fn get(&self, index: usize) -> Result<i32, String> {
        let id = self.node_at(index)?;
        Ok(self.nodes[id].as_ref().unwrap().value)
    }
    pub fn find(&self, value: i32) -> Option<usize> {
        let mut current = self.head;
        let mut index = 0;
        while let Some(id) = current {
            let node = self.nodes[id].as_ref().unwrap();
            if node.value == value { return Some(index); }
            current = node.next;
            index += 1;
        }
        None
    }
    pub fn remove_value(&mut self, value: i32) -> bool {
        match self.find(value) { Some(index) => self.erase(index).is_ok(), None => false }
    }
    pub fn values(&self, reverse: bool) -> Vec<i32> {
        let mut result = Vec::with_capacity(self.size);
        let mut current = if reverse { self.tail } else { self.head };
        while let Some(id) = current {
            let node = self.nodes[id].as_ref().unwrap();
            result.push(node.value);
            current = if reverse { node.prev } else { node.next };
        }
        result
    }
}
