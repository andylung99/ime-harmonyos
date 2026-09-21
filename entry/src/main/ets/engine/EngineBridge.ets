import engine from 'libengine.so';

export enum InputMode {
  QWERTY = 0,
  WUBI = 1,
  PINYIN = 2
}

export class Candidate {
  text: string = '';
  constructor(text: string) {
    this.text = text;
  }
}

export class EngineBridge {
  private static instance: EngineBridge = new EngineBridge();
  private sigma: number = 28;
  private userLoaded: boolean = false;

  static getInst(): EngineBridge {
    return EngineBridge.instance;
  }

  reset(): void {
    engine.reset();
  }

  setSigma(sigma: number): void {
    this.sigma = sigma;
    engine.setSigma(sigma);
  }

  feed(x: number, y: number): Candidate[] {
    const raw: string[] = engine.feed(x, y);
    return EngineBridge.toCandidates(raw);
  }

  decode(topN: number): Candidate[] {
    const raw: string[] = engine.decode(topN);
    return EngineBridge.toCandidates(raw);
  }

  backspace(): Candidate[] {
    const raw: string[] = engine.backspace();
    return EngineBridge.toCandidates(raw);
  }

  learn(word: string): void {
    engine.learn(word);
  }

  clearUserData(): void {
    engine.clearUserData();
  }

  wubiQuery(code: string, topN: number): Candidate[] {
    const raw: string[] = engine.wubiQuery(code, topN);
    return EngineBridge.toCandidates(raw);
  }

  wubiReverse(py: string, topN: number): Candidate[] {
    const raw: string[] = engine.wubiReverse(py, topN);
    return EngineBridge.toCandidates(raw);
  }

  loadWubiDict(tab: string, py: string): boolean {
    if (this.userLoaded) {
      return true;
    }
    this.userLoaded = engine.loadWubi(tab, py);
    return this.userLoaded;
  }

  private static toCandidates(raw: string[]): Candidate[] {
    const out: Candidate[] = [];
    for (let i = 0; i < raw.length; i++) {
      out.push(new Candidate(raw[i]));
    }
    return out;
  }

  getSigma(): number {
    return this.sigma;
  }
}