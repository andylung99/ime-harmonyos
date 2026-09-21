@Component
export struct CandidateBar {
  candidates: string[] = [];
  onSelect?: (index: number) => void;

  build() {
    Scroll() {
      Row({ space: 8 }) {
        ForEach(this.candidates, (item: string, idx: number) => {
          Button(item)
            .fontSize(16)
            .height(36)
            .backgroundColor('#EFF3F8')
            .fontColor('#0A59F7')
            .onClick(() => {
              this.onSelect?.(idx);
            })
        }, (item: string) => item)
      }
      .padding({ left: 8, right: 8, top: 6, bottom: 6 })
    }
    .width('100%')
    .height(48)
    .scrollable(ScrollDirection.Horizontal)
    .backgroundColor('#FFFFFF');
  }
}