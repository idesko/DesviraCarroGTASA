# Swap Andromada de Cutscene por Trator

## O que foi feito

Foi criado um override loose no `modloader` para trocar o visual do `androm` da cutscene pelo visual do `tractor`.

Pasta ativa:

- `modloader/Andromada Tractor Cutscene`

Arquivos ativos:

- `modloader/Andromada Tractor Cutscene/models/gta3img/androm.dff`
- `modloader/Andromada Tractor Cutscene/models/gta3img/androm.txd`

## Fonte do modelo

O modelo base veio do mod existente:

- `modloader/Tractor Wheel Giant/models/gta3img/tractor.dff`
- `modloader/Tractor Wheel Giant/models/gta3img/tractor.txd`

Esses arquivos foram copiados e renomeados para `androm.*`, para que o Mod Loader substitua o modelo da cutscene sem mexer no `gta3.img`.

## Por que essa abordagem

- evita editar o archive principal do jogo
- fica facil ligar e desligar apagando a pasta
- reaproveita um modelo que ja estava ativo e funcionando no modloader
- reduz risco de quebrar outros swaps ou extracoes do IMG

## Como isso funciona no jogo

Quando o jogo pede o modelo `androm`, o Mod Loader encontra os arquivos loose desta pasta e entrega o visual do `tractor` no lugar.

Em outras palavras:

- nome esperado pelo jogo: `androm`
- visual fornecido pelo mod: `tractor`

## Como desfazer

Para remover a troca, basta apagar:

- `modloader/Andromada Tractor Cutscene`

O resto do jogo permanece intacto.

## Observacoes

- Este arquivo e apenas documentacao.
- O mod ativo fica separado da documentacao, conforme combinado.
- Se for necessario, a mesma tecnica pode ser usada para outros swaps de cutscene.
