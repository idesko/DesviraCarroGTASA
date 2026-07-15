# Guia de Referencia para Criacao de Mods GTA SA

Fontes lidas:
- [1. Introducao e DFF](https://forum.mixmods.com.br/f37-tutoriais/t868-1-introducao-e-dff)
- [2. Como abrir um carro do GTA SA](https://forum.mixmods.com.br/f37-tutoriais/t870-2-como-abrir-um-carro-do-gta-sa)
- [Lista de dummy/frames e suas funcoes](https://forum.mixmods.com.br/f38-utilidades/t869-lista-de-dummy-frames-e-suas-funcoes)
- [3. Dummy, coordenada, rotacao](https://forum.mixmods.com.br/f37-tutoriais/t871-3-dummy-coordenada-rotacao)  
  Observacao: este link retornou 403 no acesso direto, entao o resumo desta parte foi inferido a partir do indice do tutorial e dos conceitos apresentados nas partes anteriores.

## 1. O que um DFF e no GTA SA

- Um `.dff` e um arquivo RenderWare.
- A ideia principal e pensar nele como uma hierarquia.
- O modelo nao e so uma malha solta: ele tem `clump`, `frame` e `atomic`.
- `Atomic` e a parte visual da geometria.
- `Frame` guarda posicao, rotacao e tamanho.
- `Clump` e a raiz que organiza frames e atomics.

Ponto importante:
- Quando voce mexe em um modelo, normalmente voce nao quer so trocar a forma visual.
- Voce precisa respeitar a hierarquia, porque e ela que decide onde rodas, portas, asas, rotores e outras partes vao parar.

## 2. Abrindo um carro do GTA SA

- Em modelos originais, o `dummy root` representa a origem do modelo.
- Esse root precisa estar bem posicionado e rotacionado.
- No ZModeler, o nome do dummy/export root costuma ser usado na exportacao do arquivo.
- Se o carro estiver bloqueado para importacao, o tutorial cita abrir no RW Analyze e usar `Ctrl+D` e `Ctrl+S` para destravar.

Resumo pratico:
- Sempre confira a arvore do modelo antes de exportar.
- Se o root estiver errado, o carro pode nascer torto, afundado ou com partes deslocadas.
- O nome do arquivo nao e o mais importante; a estrutura da hierarquia e a posicao dos frames sao.

## 3. Dummy, coordenada e rotacao

Resumo inferido da sequencia do tutorial:
- `Dummy` nao e geometria, e um marcador de posicao/rotacao.
- A coordenada do dummy define onde uma parte do modelo vai nascer no mundo.
- A rotacao do dummy define como essa parte se orienta.
- Em veiculos, o pai e os filhos importam muito: deslocar um frame pai afeta tudo que esta embaixo dele.
- O alinhamento correto evita roda fora do eixo, porta no lugar errado e modelo "deitado".

Regra pratica:
- Se uma parte precisa se mover junto com o restante, ela deve estar no frame certo.
- Se a orientacao de um node esta errada, a parte pode girar em eixo errado ou se comportar invertido.

## 4. Lista de dummy/frames mais importantes

### Tipos gerais

- `chassis_dummy`: inclui todo o veiculo exceto as rodas. E nele que a inclinacao da carroceria com a suspensao e aplicada.
- `chassis`: frame principal da carroceria.
- `_vlo`: LOD do veiculo. So o ultimo lido costuma valer.
- `windscreen*`: aplica correcao de alpha/transparencia.

### Tipo `car`

- `wheel_rf_dummy`, `wheel_rm_dummy`, `wheel_rb_dummy`, `wheel_lf_dummy`, `wheel_lm_dummy`, `wheel_lb_dummy`: posicoes das rodas.
- `door_rf_dummy`, `door_rr_dummy`, `door_lf_dummy`, `door_lr_dummy`: posicoes das portas.
- `bump_front_dummy`, `bump_rear_dummy`: para-choques.
- `wing_rf_dummy`, `wing_lf_dummy`: para-lamas.
- `bonnet_dummy`: capo.
- `boot_dummy`: tampa do porta-malas.
- `windscreen_dummy`: para-brisa.
- `exhaust`: fumaça do escapamento.
- `engine`: fumaça do motor.
- `petrolcap`: entrada de combustivel.
- `hookup`: ponto de engate.
- `misc_a`, `misc_b`, `misc_c`, `misc_d`, `misc_e`: funcoes hardcoded ou dummies de posicao.
- `ped_frontseat`, `ped_backseat`: posicoes de ocupantes.
- `extra1` a `extra6`: extras do carro.
- `ug_*`: dummies de tuning, como `ug_bonnet`, `ug_spoiler`, `ug_roof`, `ug_nitro`.

### Tipo `plane`

- `chassis`: corpo principal.
- `static_prop`, `moving_prop`, `static_prop2`, `moving_prop2`: helices.
- `rudder`: leme.
- `elevator_l`, `elevator_r`: profundor.
- `aileron_l`, `aileron_r`: ailerons.
- `gear_l`, `gear_r`: pecas adicionais usadas em alguns modelos.
- `aileron_pos`, `elevator_pos`, `rudder_pos`: pontos usados para danos.
- `wingtip_pos`: posicao do efeito de trilha nas pontas das asas.
- `ped_frontseat`, `ped_backseat`: ocupantes.
- `miscpos_a`, `miscpos_b`, `extra1` a `extra6`: dummies auxiliares.

Nota util:
- O tutorial avisa que o Dodo usa o nome `elevator` de forma errada, mas ainda assim funciona como parte de voo.

### Tipo `heli`

- `chassis`: corpo principal.
- `static_rotor`, `moving_rotor`, `static_rotor2`, `moving_rotor2`: rotores.
- `rudder`: leme.
- `elevators`: equivalentes aos `elevator_*`.
- `misc_a`, `misc_b`, `misc_c`, `misc_d`: auxiliares.
- `ped_frontseat`, `ped_backseat`: ocupantes.
- `headlights`, `taillights`, `headlights2`, `taillights2`: luzes.
- `exhaust`: saida de escapamento.

### Tipo `bike`

- `forks_front`: garfo dianteiro.
- `forks_rear`: garfo traseiro.
- `wheel_front`: roda da frente.
- `wheel_rear`: roda de tras.
- `mudguard`: para-lama.
- `handlebars`: guidao.
- `bargrip`: posicao de agarre da mao direita.
- `ped_frontseat`, `ped_backseat`: ocupantes.

### Tipo `bmx`

- `forks_front`, `forks_rear`, `wheel_front`, `wheel_rear`, `handlebars`.

## 5. Como pensar a hierarquia

- `node`, `frame` e `dummy` sao, na pratica, a mesma lingua de trabalho para quem edita modelos.
- `root` e a origem de tudo.
- Um node pai controla os filhos.
- Se o frame pai estiver deslocado, os filhos vao junto.
- Se o frame pai estiver rotacionado, o conjunto inteiro muda de orientacao.

Isso explica varios bugs comuns:
- roda fora do lugar
- porta travada
- aerofolio torto
- parte do modelo afundando
- extra aparecendo na posicao errada

## 6. Workflow recomendado para criar ou corrigir um mod

1. Abra o modelo original no editor.
2. Confira se o `root` esta na posicao e rotacao corretas.
3. Verifique se cada parte esta no frame esperado.
4. Compare o nome dos dummies com a funcao que o GTA espera para aquele tipo de veiculo.
5. Ao editar, prefira mover frames corretamente em vez de so "empurrar" a malha.
6. Exporte e teste no jogo.
7. Se algo ficar desalinhado, volte e revise a hierarquia antes de mexer em textura.

## 7. Regras praticas que ajudam muito

- Dummy nao e geometria. Ele so marca posicao e orientacao.
- Root errado geralmente estraga o modelo inteiro.
- Em veiculos, nomes importam muito mais do que parece.
- Ao mexer em `frames`, pense sempre em pai/filho.
- Se um detalhe precisa acompanhar outro, ele provavelmente esta no lugar errado da arvore.
- Se voce quer "escala visual", mexer no frame raiz e geralmente mais seguro do que editar a colisao.

## 8. Resumo rapido para lembrar

- `.dff` = hierarquia RenderWare com `clump`, `frames` e `atomics`.
- `frame` guarda posicao, rotacao e tamanho.
- `dummy` e um frame especial usado como marcador.
- O nome do dummy pode ativar funcoes hardcoded do GTA.
- O tipo do veiculo define quais dummies fazem sentido.
- O segredo quase sempre e hierarquia correta, nao so geometria bonita.

## 9. Observacoes finais

- Este arquivo e uma memoria de trabalho para criacao de mods.
- Ele nao substitui o tutorial completo, mas junta as partes mais uteis para consulta rapida.
- Se voce for mexer em outro modelo depois, vale revisar primeiro a lista de dummies do tipo certo do veiculo.
