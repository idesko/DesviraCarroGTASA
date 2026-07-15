# DesviraCarroGTASA

Mods e testes para GTA San Andreas com foco em comportamento de carro capotado, CLEO, ASI e documentação de trabalho.

## O que tem aqui

- `flipped_car_rescue_cleo/`: mod CLEO para evitar fogo/explosão ao capotar e tentar recuperar o carro com `W/A/S/D`
- `tiny_peds_asi/`: experimento para reduzir pedestres
- `rocket_man_asi/`: experimento de NPC com jetpack
- `rc_pilot_asi/` e `rc_pilot_test/`: testes de RC/cutscene
- `train_3_vagoes_cleo/`: experimento com trem
- `modloader/criador_de_mods/`: documentação e handoffs para futuras IAs

## Estado atual

O mod ativo mais relevante no momento é o `Flipped Car Rescue - WASD`.

Ele faz duas coisas:

- aplica o patch anti-fogo de carro capotado
- usa um delay de estabilidade antes de aceitar a tecla, tentando ajudar o carro a voltar com torque e handling

## Uso

Os arquivos dentro desta pasta servem como base de trabalho e documentação.  
As pastas com `modloader/` representam o estado ativo no jogo.

## Nota

Este repositório foi organizado para facilitar testes rápidos, documentação curta e handoff para outra IA sem perder o contexto.
