export interface KeyItem {
  id: string;
  label: string;
  col: number;
  row: number;
  wRatio: number;
}

export const KEY_H = 50;

export function buildKeyLayout(): KeyItem[] {
  const items: KeyItem[] = [];
  const rows: string[][] = [
    ['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'],
    ['a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'],
    ['z', 'x', 'c', 'v', 'b', 'n', 'm'],
    ['_FUNC_SPACE', '_FUNC_ENTER', '_FUNC_BACKSPACE']
  ];
  for (let r = 0; r < rows.length; r++) {
    const row = rows[r];
    const startCol = r === 0 ? 0 : r === 1 ? 0 : r === 2 ? 1 : 2;
    for (let c = 0; c < row.length; c++) {
      items.push({
        id: row[c],
        label: row[c] === '_FUNC_SPACE' ? '空格' : row[c] === '_FUNC_ENTER' ? '回车' : row[c] === '_FUNC_BACKSPACE' ? '退格' : row[c].toUpperCase(),
        col: startCol + c,
        row: r,
        wRatio: 1
      });
    }
  }
  return items;
}