export interface EngineNative {
  loadWubi(tab: string, py: string): boolean;
  wubiQuery(code: string, topN: number): string[];
  wubiReverse(py: string, topN: number): string[];
  reset(): void;
  backspace(): string[];
  feed(x: number, y: number): string[];
  decode(topN: number): string[];
  learn(word: string): void;
  setSigma(sigma: number): void;
  clearUserData(): void;
}

declare module 'libengine.so' {
  const engine: EngineNative;
  export default engine;
}