import { KeyboardController } from '@kit.InputMethodKit';
import { EngineBridge, Candidate } from '../engine/EngineBridge';
import { KeyboardView } from '../keyboard/KeyboardView';
import { CandidateBar } from '../keyboard/CandidateBar';

@Entry
@Component
struct KeyboardPage {
  @StorageLink('imeMode') imeMode: string = 'pinyin';
  @State candidates: string[] = [];
  private wubiBuf: string = '';

  private getController(): KeyboardController | undefined {
    return AppStorage.get<KeyboardController>('keyboardController');
  }

  private onKeyTap(id: string, x: number, y: number): void {
    if (id.startsWith('_FUNC_')) {
      this.handleFunc(id);
      return;
    }
    if (this.imeMode === 'wubi') {
      this.wubiBuf = `${this.wubiBuf}${id}`;
      const cands = EngineBridge.getInst().wubiQuery(this.wubiBuf, 8);
      this.candidates = cands.map((c: Candidate) => c.text);
      return;
    }
    const cands = EngineBridge.getInst().feed(x, y);
    this.candidates = cands.map((c: Candidate) => c.text);
  }

  private handleFunc(id: string): void {
    const controller = this.getController();
    const bridge = EngineBridge.getInst();
    if (id === '_FUNC_SPACE') {
      if (this.candidates.length > 0) {
        this.selectCandidate(0);
        return;
      }
      controller?.insertText({ text: ' ' });
      return;
    }
    if (id === '_FUNC_BACKSPACE') {
      controller?.deleteBackward({ length: 1 });
      const cands = bridge.backspace();
      this.candidates = cands.map((c: Candidate) => c.text);
      return;
    }
    if (id === '_FUNC_ENTER') {
      controller?.sendKeyFunction({ keyFuncType: 0 });
    }
  }

  private selectCandidate(index: number): void {
    const controller = this.getController();
    const candidate = this.candidates[index];
    if (!candidate) {
      return;
    }
    const bridge = EngineBridge.getInst();
    bridge.learn(candidate);
    controller?.insertText({ text: candidate });
    if (this.imeMode === 'wubi') {
      this.wubiBuf = '';
      const cands = bridge.wubiQuery('', 8);
      this.candidates = cands.map((c: Candidate) => c.text);
      return;
    }
    this.candidates = [];
    bridge.reset();
  }

  build() {
    Column() {
      CandidateBar({
        candidates: this.candidates,
        onSelect: (index: number) => this.selectCandidate(index)
      })
      KeyboardView({ onTap: (id: string, x: number, y: number) => this.onKeyTap(id, x, y) })
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF');
  }
}