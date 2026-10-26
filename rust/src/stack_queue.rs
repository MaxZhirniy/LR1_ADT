use crate::forward_list::ForwardList;
#[derive(Default)]
pub struct Stack { items: ForwardList }
impl Stack {
    pub fn push(&mut self, value: String) { self.items.insert(0, value).unwrap(); }
    pub fn pop(&mut self) -> Result<String, String> { self.items.erase(0) }
    pub fn top(&self) -> Result<String, String> { self.items.get(0) }
    pub fn values(&self) -> Vec<String> { self.items.values(false) }
}
pub struct Queue {
    buf: Vec<Option<String>>,
    head: usize,
    size: usize,
}
impl Default for Queue {
    fn default() -> Self { Self { buf: vec![None; 4], head: 0, size: 0 } }
}
impl Queue {
    fn grow(&mut self) {
        let old = self.buf.len();
        let mut next = vec![None; old * 2];
        for (i, slot) in next.iter_mut().enumerate().take(self.size) {
            *slot = self.buf[(self.head + i) % old].take();
        }
        self.buf = next;
        self.head = 0;
    }
    pub fn push(&mut self, value: String) {
        if self.size == self.buf.len() { self.grow(); }
        let index = (self.head + self.size) % self.buf.len();
        self.buf[index] = Some(value);
        self.size += 1;
    }
    pub fn pop(&mut self) -> Result<String, String> {
        if self.size == 0 { return Err("queue is empty".into()); }
        let value = self.buf[self.head].take().unwrap();
        self.head = (self.head + 1) % self.buf.len();
        self.size -= 1;
        Ok(value)
    }
    pub fn front(&self) -> Result<String, String> {
        if self.size == 0 { return Err("queue is empty".into()); }
        Ok(self.buf[self.head].as_ref().unwrap().clone())
    }
    pub fn values(&self) -> Vec<String> {
        (0..self.size).map(|i| self.buf[(self.head + i) % self.buf.len()].as_ref().unwrap().clone()).collect()
    }
}
