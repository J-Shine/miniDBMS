SELECT t2.hometown, cp.nickname
FROM Trainer AS t2, CatchedPokemon AS cp, (
  SELECT t3.hometown, MAX(level) AS ml
  FROM Trainer AS t3, CatchedPokemon AS cp2
  WHERE t3.id = cp2.owner_id
  GROUP BY t3.hometown) AS t1
WHERE t2.id = cp.owner_id
AND t1.hometown = t2.hometown
AND cp.level = t1.ml
ORDER BY t2.hometown