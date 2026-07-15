# Harness de GTA3Script / CLEO para GTA SA

Este arquivo junta os dois mundos que mais ajudam a criar mods no GTA San Andreas:

- modelos/DFF/dummies/frame hierarchy
- scripts CLEO/GTA3Script para ativar, controlar e testar mods no jogo

Use este arquivo como guia de trabalho para outra IA ou como mapa rapido para montar mods com menos tentativa e erro.

## Fontes-base

- [1. Introducao e DFF](https://forum.mixmods.com.br/f37-tutoriais/t868-1-introducao-e-dff)
- [2. Como abrir um carro do GTA SA](https://forum.mixmods.com.br/f37-tutoriais/t870-2-como-abrir-um-carro-do-gta-sa)
- [Lista de dummy/frames e suas funcoes](https://forum.mixmods.com.br/f38-utilidades/t869-lista-de-dummy-frames-e-suas-funcoes)
- [3. Dummy, coordenada, rotacao](https://forum.mixmods.com.br/f37-tutoriais/t871-3-dummy-coordenada-rotacao)
- [Indice de tutoriais - CLEO Script - GTA3script](https://forum.mixmods.com.br/f141-gta3script-cleo/t26-indice-de-tutoriais-cleo-script-gta3script)
- [Como criar scripts com CLEO+](https://forum.mixmods.com.br/f141-gta3script-cleo/t5206-como-criar-scripts-com-cleo)

## Objetivo deste harness

Quando a IA for criar um mod, ela deve pensar nesta ordem:

1. O que sera alterado no jogo?
2. Isso e um modelo, uma animacao, uma entidade, uma coordenada ou um script?
3. Se for modelo, quais dummies/frames o GTA espera?
4. Se for script, qual evento dispara a acao?
5. Como validar no jogo sem depender de chute?

## Mapa rapido do aprendizado GTA3Script

O indice da MixMods organiza o aprendizado assim:

1. Introducao ao tutorial
2. Preparacoes
3. Visual Studio Code
4. Seu primeiro script cleo
5. Comentarios
6. Principios de funcionamento e loops
7. Condicoes
8. Bits & Bytes
9. Variaveis e operadores
10. Exercicio #1
11. Comandos e argumentos
12. Escopos
13. GOSUB e CLEO_CALL
14. Indentacao
15. Mais sobre condicoes
16. Arrays
17. Exercicio #2
18. Tipos de entidades
19. REQUIRE

Depois disso, o indice ainda separa:

- manipulacao de memoria
- como fazer coisas praticas
- utilidades

## Como usar esse mapa de forma pratica

- Para scripts simples, comece em `1` a `11`.
- Para mods que interagem com peds, carros e objetos, leia com carinho `18. Tipos de entidades`.
- Para dependencias e organizacao do script, `19. REQUIRE` ajuda muito.
- Para mod mais avancado, a parte de memoria vira a ponte para sistemas mais complexos.

## O que a IA precisa lembrar sobre CLEO e GTA3Script

- CLEO e o motor do script.
- GTA3Script e a sintaxe/tela de producao do script.
- O modulo mais importante nao e decorar opcode, e sim saber quando usar cada coisa.
- Comentarios bem escritos economizam horas na proxima iteracao.
- Teste no jogo real sempre vale mais que teoria bonita.

## Resumo do CLEO+

O artigo de CLEO+ e importante porque ele amplia muito o que da para fazer em CLEO.

Pontos que valem guardar:

- CLEO+ adiciona muitos comandos novos e prontos para uso.
- Ele foi pensado para facilitar mods mais estaveis e mais práticos.
- O indice da MixMods recomenda aprender GTA3Script antes de mergulhar pesado em CLEO+.
- O tutorial destaca recursos utilissimos como:
  - criar scripts independentes
  - compartilhar memoria entre scripts
  - criar eventos para entidades
  - criar objetos e geradores sem salvar no game

## Tarefas que a IA deve priorizar ao criar mods

### 1. Verificar o estado real do jogo

- Antes de afirmar que algo funcionou, confira o estado ativo.
- Em GTA SA modded, `modloader.log` e os arquivos realmente carregados valem mais que a intencao do script.

### 2. Reaproveitar o que ja existe

- Se ja existir um script ou setup funcional, prefira adaptar ele.
- Reusar base boa e mais seguro que reinventar tudo do zero.

### 3. Separar modelo de script

- DFF controla forma, hierarquia e dummies.
- CLEO/GTA3Script controla comportamento.
- Se o problema e visual, olhe o modelo.
- Se o problema e disparo/acao/tempo, olhe o script.

### 4. Trabalhar com entidades corretas

- Ped, car, object, train, vehicle e custom script nao sao a mesma coisa.
- Um script bom respeita o tipo da entidade antes de tentar modificar comportamento.

## Linha de raciocinio para mods de exemplo

### Se o pedido for de modelo

- descobrir o modelo original
- conferir dummies corretos
- entender root, frame e rotacao
- validar no jogo

### Se o pedido for de script

- definir tecla ou evento
- carregar modelo com seguranca
- criar entidade
- controlar movimento ou estado
- limpar referencias depois

### Se o pedido for misto

- usar DFF para encaixe visual
- usar GTA3Script para ativacao e logica
- testar cada parte separadamente

## Regras de ouro para nao se perder

- dummy nao e geometria
- frame pai afeta filho
- root errado bagunca o modelo inteiro
- comando certo sem contexto ainda pode gerar bug
- script bom sem verificacao de estado costuma parecer funcionar e falhar depois

## Guia de trabalho recomendado

1. Ler o objetivo do mod.
2. Descobrir se o problema e de modelo, entidade ou script.
3. Puxar os dummies/frames certos do modelo.
4. Definir o fluxo do GTA3Script.
5. Compilar.
6. Testar no jogo.
7. Documentar o que funcionou e o que nao funcionou.

## Tatico para outra IA

Se outra IA for continuar daqui, ela deve receber:

- este harness
- o guia de DFF/dummies
- a pasta ativa do modloader
- o estado real do `modloader.log`
- o objetivo exato do mod

Assim ela nao precisa adivinhar:

- nomes de frames
- nomes de dummies
- se o script esta ativo
- se o problema esta no modelo ou na logica

## Arquivo complementar

O guia anterior, focado em DFF e dummies, continua sendo o complemento ideal deste arquivo:

- `GUIA-MIXMODS-DUMMIES-DFF.md`

Juntos, eles cobrem:

- estrutura do modelo
- pontos de encaixe
- comandos e fluxo de scripts
- leitura de tutorial
- abordagem de teste

## Conclusao curta

Se a meta e criar mods com menos erro, a combinacao certa e:

- DFF/dummies para a estrutura
- GTA3Script/CLEO para comportamento
- CLEO+ para ampliar as possibilidades
- verificacao real no jogo para confirmar o resultado
