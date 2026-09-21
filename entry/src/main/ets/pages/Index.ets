import { preferences } from '@kit.ArkData';
import { common } from '@kit.AbilityKit';
import { EngineBridge } from '../engine/EngineBridge';

@Entry
@Component
struct Index {
  @State fuzzy: boolean = false;
  @State mode: string = 'pinyin';
  @State status: string = '';

  private getPrefs(): preferences.Preferences {
    const ctx = getContext(this) as common.Context;
    return preferences.getPreferencesSync(ctx, { name: 'ime_settings' });
  }

  aboutToAppear(): void {
    const p = this.getPrefs();
    this.fuzzy = p.getSync('fuzzy', false) as boolean;
    this.mode = p.getSync('mode', 'pinyin') as string;
    AppStorage.setOrCreate<string>('imeMode', this.mode);
    EngineBridge.getInst().setSigma(this.fuzzy ? 40 : 28);
  }

  private onFuzzyChange(checked: boolean): void {
    this.fuzzy = checked;
    const p = this.getPrefs();
    p.putSync('fuzzy', checked);
    p.flush();
    EngineBridge.getInst().setSigma(checked ? 40 : 28);
    this.status = '模糊音已' + (checked ? '开启' : '关闭');
  }

  private onModeChange(name: string): void {
    this.mode = name;
    const p = this.getPrefs();
    p.putSync('mode', name);
    p.flush();
    AppStorage.setOrCreate<string>('imeMode', name);
    this.status = '已切换为' + (name === 'wubi' ? '五笔' : '拼音');
  }

  private clearUser(): void {
    EngineBridge.getInst().clearUserData();
    this.status = '用户词库已清除';
  }

  @Builder
  modeButton(name: string, label: string) {
    Button(label)
      .width(96)
      .height(40)
      .backgroundColor(this.mode === name ? '#0A59F7' : '#EEEEEE')
      .fontColor(this.mode === name ? '#FFFFFF' : '#111111')
      .onClick(() => this.onModeChange(name));
  }

  build() {
    Column({ space: 16 }) {
      Text('自研输入法')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 32 })
      Text('安装后请到「系统设置 > 系统和更新 > 语言和输入法」中启用本输入法，并在任意输入框长按弹出菜单中切换。')
        .fontSize(14)
        .fontColor('#666666')
        .padding({ left: 16, right: 16 })
      Text('输入模式')
        .fontSize(16)
      Row({ space: 8 }) {
        this.modeButton('pinyin', '拼音')
        this.modeButton('wubi', '五笔')
      }
      Row() {
        Text('模糊音（n/l、z/zh、an/ang 等，按键容错更高）')
          .fontSize(14)
          .layoutWeight(1)
        Toggle({ type: ToggleType.Switch, isOn: this.fuzzy })
          .onChange((isOn: boolean) => this.onFuzzyChange(isOn))
      }
      .width('90%')
      Button('清除用户词库')
        .width('90%')
        .onClick(() => this.clearUser())
      Text(this.status)
        .fontSize(13)
        .fontColor('#0A59F7')
      Blank()
    }
    .alignItems(HorizontalAlign.Center)
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF');
  }
}