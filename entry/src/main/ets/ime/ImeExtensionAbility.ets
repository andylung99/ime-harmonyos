import { InputMethodExtensionAbility, InputMethodExtensionContext,
  KeyboardController } from '@kit.InputMethodKit';
import { resourceManager } from '@kit.LocalizationKit';
import { util } from '@kit.ArkTS';
import { EngineBridge } from '../engine/EngineBridge';

const TAG = 'ImeExtAbility';

function bytesToString(buf: Uint8Array): string {
  const decoder = new util.TextDecoder();
  return decoder.decodeToString(buf);
}

export default class ImeExtensionAbility extends InputMethodExtensionAbility {
  private controller?: KeyboardController;

  onCreate(context: InputMethodExtensionContext): void {
    super.onCreate(context);
    const controller = context.getKeyboardController();
    this.controller = controller;
    AppStorage.setOrCreate<KeyboardController>('keyboardController', controller);
    AppStorage.setOrCreate<InputMethodExtensionContext>('imeContext', context);
    this.prepareDict(context);
    this.loadContent('pages/KeyboardPage');
  }

  private async prepareDict(context: InputMethodExtensionContext): Promise<void> {
    try {
      const tab = await this.readRawFile(context, 'wubi_sample.txt');
      const py = await this.readRawFile(context, 'pinyin_sample.txt');
      EngineBridge.getInst().loadWubiDict(tab, py);
    } catch (e) {
      console.error(`${TAG} load dict failed: ${JSON.stringify(e)}`);
    }
  }

  private async readRawFile(context: InputMethodExtensionContext, name: string): Promise<string> {
    const mgr: resourceManager.ResourceManager = context.resourceManager;
    const data: Uint8Array = await mgr.getRawFileContent(name);
    return bytesToString(data);
  }

  onDestroy(): void {
    this.controller = undefined;
    super.onDestroy();
  }
}