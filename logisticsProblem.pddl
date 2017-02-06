(define (problem logisticsProblem0)
(:domain logistics_PDDL)
(:objects
	area0
	area1
	food
	door1-0
	key1-0 
	)

(:init
(npc-at area0)
(npc-not-close-to-point)

(waypoint door1-0)
(point-of-interest door1-0 area1)
(point-of-interest door1-0 area0)
(connected area1 area0 door1-0)
(connected area0 area1 door1-0)
(closed door1-0)

(point-of-interest key1-0 area0)
(item key1-0)
(key key1-0 door1-0)

(item food)
(point-of-interest food area1)

)

(:goal
(npc-holding food)
;(open door1-0)
;(npc-at area1)
)

)
