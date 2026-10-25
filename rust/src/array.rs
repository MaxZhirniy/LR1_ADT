/// Динамический массив: индексируемые операции и проверка границ.
#[derive(Default)]
pub struct DynamicArray {
    items: Vec<i32>,
}

impl DynamicArray {
    pub fn len(&self) -> usize { self.items.len() }
    pub fn values(&self) -> Vec<i32> { self.items.clone() }
    pub fn push(&mut self, value: i32) { self.items.push(value); }
    pub fn insert(&mut self, index: usize, value: i32) -> Result<(), String> {
        if index > self.len() { return Err("index out of range".into()); }
        self.items.insert(index, value);
        Ok(())
    }
    pub fn get(&self, index: usize) -> Result<i32, String> {
        self.items.get(index).copied().ok_or_else(|| "index out of range".into())
    }
    pub fn set(&mut self, index: usize, value: i32) -> Result<(), String> {
        let slot = self.items.get_mut(index).ok_or("index out of range")?;
        *slot = value;
        Ok(())
    }
    pub fn erase(&mut self, index: usize) -> Result<i32, String> {
        if index >= self.len() { return Err("index out of range".into()); }
        Ok(self.items.remove(index))
    }
}
